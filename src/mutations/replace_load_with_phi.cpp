#pragma once
#include <vector>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "mutations/mutation.h"
#include "utils/is_optimizable.cpp"
#include "utils/utils.cpp"
using namespace llvm;

static void collectLiveAtEndOf(Function *F, const DominatorTree &DT,
                                BasicBlock *pred, Type *ty,
                                std::vector<Value *> &out) {
    for (Argument &A : F->args())
        if (A.getType() == ty)
            out.push_back(&A);

    for (BasicBlock &B : *F) {
        if (!DT.dominates(&B, pred)) continue;
        for (Instruction &I : B) {
            if (I.isTerminator()) continue;
            if (I.getType() != ty) continue;
            out.push_back(&I);
        }
    }
}

// Picks any non-volatile load that is not in the entry block and replaces it
// with a PHI node.  For each predecessor of the load's block one same-typed
// value that is live at the end of that predecessor is chosen independently.
class ReplaceLoadWithPhi : public Mutation {
public:
    ReplaceLoadWithPhi() : Mutation() {}
    ReplaceLoadWithPhi(std::vector<int> decisions) : Mutation(std::move(decisions)) {}

    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        std::vector<Function *> fns;
        for (Function &F : *M)
            if (!F.isDeclaration() && isOptimizable(&F))
                fns.push_back(&F);

        if (fns.empty()) {
            errs() << "LOAD_TO_PHI: no optimizable functions.\n";
            return nullptr;
        }

        Function *F = fns[dm.make_decision(0, fns.size() - 1)];

        DominatorTree DT;
        DT.recalculate(*F);

        // Any non-volatile load outside the entry block.
        std::vector<LoadInst *> loads;
        BasicBlock *entry = &F->getEntryBlock();
        for (BasicBlock &BB : *F) {
            if (&BB == entry) continue;
            for (Instruction &I : BB)
                if (auto *LI = dyn_cast<LoadInst>(&I))
                    if (!LI->isVolatile())
                        loads.push_back(LI);
        }

        if (loads.empty()) {
            errs() << "LOAD_TO_PHI: no eligible load found.\n";
            return nullptr;
        }

        LoadInst *target = loads[dm.make_decision(0, loads.size() - 1)];
        BasicBlock *joinBlock = target->getParent();
        Type *ty = target->getType();

        SmallVector<BasicBlock *, 8> preds(pred_begin(joinBlock), pred_end(joinBlock));

        // For each predecessor, independently pick a same-typed value that is
        // live at the end of that predecessor.  Each gets its own decision.
        IRBuilder<> builder(&*joinBlock->getFirstInsertionPt());
        PHINode *phi = builder.CreatePHI(ty, preds.size(), "load_phi");

        for (BasicBlock *pred : preds) {
            std::vector<Value *> cands;
            collectLiveAtEndOf(F, DT, pred, ty, cands);

            if (cands.empty()) {
                phi->eraseFromParent();
                errs() << "LOAD_TO_PHI: no live same-typed value for a predecessor.\n";
                return nullptr;
            }

            Value *chosen = cands[dm.make_decision(0, cands.size() - 1)];
            phi->addIncoming(chosen, pred);
        }

        target->replaceAllUsesWith(phi);

        if (target->use_empty())
            target->eraseFromParent();

        return std::move(M);
    }
};
