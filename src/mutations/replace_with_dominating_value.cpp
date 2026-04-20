#pragma once
#include <vector>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/raw_ostream.h"
#include "mutations/mutation.h"
#include "utils/is_optimizable.cpp"
#include "utils/utils.cpp"
using namespace llvm;

// Collect all values that have the same type as `ty` and are defined
// before `point` in the dominator order
static void collectRAUWCandidates(Function *F, const DominatorTree &DT,
                                  Instruction *point, Type *ty,
                                  std::vector<Value *> &out) {
    for (Argument &A : F->args()) {
        if (A.getType() == ty)
            out.push_back(&A);
    }
    for (BasicBlock &BB : *F) {
        for (Instruction &I : BB) {
            if (&I == point) continue;
            if (I.getType() != ty) continue;
            if (!DT.dominates(&I, point)) continue;
            out.push_back(&I);
        }
    }
}

// Picks a non-void instruction that has at least one use, finds another
// value of the same type that dominates it, then calls replaceAllUsesWith().
// If the original instruction becomes dead and side-effect-free it is erased.
class ReplaceWithDominatingValue : public Mutation {
public:
    ReplaceWithDominatingValue() : Mutation() {}
    ReplaceWithDominatingValue(std::vector<int> decisions) : Mutation(std::move(decisions)) {}

    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        std::vector<Function *> fns;
        for (Function &F : *M) {
            if (!F.isDeclaration() && isOptimizable(&F))
                fns.push_back(&F);
        }
        if (fns.empty()) {
            errs() << "RAUW: no optimizable functions.\n";
            return nullptr;
        }

        Function *F = fns[dm.make_decision(0, fns.size() - 1)];

        // Collect candidate targets: non-void, non-terminator instructions
        // that actually have uses (otherwise RAUW is a no-op).
        std::vector<Instruction *> targets;
        for (BasicBlock &BB : *F) {
            for (Instruction &I : BB) {
                if (I.isTerminator()) continue;
                if (I.getType()->isVoidTy()) continue;
                if (I.use_empty()) continue;
                targets.push_back(&I);
            }
        }
        if (targets.empty()) {
            errs() << "RAUW: no candidate target instructions.\n";
            return nullptr;
        }

        Instruction *target = targets[dm.make_decision(0, targets.size() - 1)];
        Type *ty = target->getType();

        DominatorTree DT;
        DT.recalculate(*F);

        std::vector<Value *> replacements;
        collectRAUWCandidates(F, DT, target, ty, replacements);

        if (replacements.empty()) {
            errs() << "RAUW: no same-type dominating value found for replacement.\n";
            return nullptr;
        }

        Value *replacement = replacements[dm.make_decision(0, replacements.size() - 1)];

        target->replaceAllUsesWith(replacement);

        // Erase target if it is now dead and has no side effects.
        if (target->use_empty() && !target->mayHaveSideEffects())
            target->eraseFromParent();

        return std::move(M);
    }
};
