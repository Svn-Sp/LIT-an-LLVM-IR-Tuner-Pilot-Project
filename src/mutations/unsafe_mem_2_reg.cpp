#pragma once
#include <vector>
#include "llvm/IR/Instructions.h"
#include <fstream>
#include <random>
#include <stdio.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "mutations/mutation.h"
#include "llvm/IR/Dominators.h"
#include "utils/utils.cpp"
#include "utils/randomness_utils.cpp"
#include "utils/is_optimizable.cpp"
using namespace llvm;

bool simpleIsAllocaPromotable(AllocaInst *AI) {
    std::vector<const Value *> worklist;
    worklist.push_back(AI);
    while (!worklist.empty()) {
        const Value *V = worklist.back();
        worklist.pop_back();
        for (const Use &U : V->uses()) {
            const User *Usr = U.getUser();
            if (const LoadInst *LI = dyn_cast<LoadInst>(Usr)) {
                continue;
            }
            if (const StoreInst *SI = dyn_cast<StoreInst>(Usr)) {
                if (SI->getPointerOperand() == V) {
                    continue;
                }
                return false;
            }
            if (isa<GetElementPtrInst>(Usr) || isa<BitCastInst>(Usr)) {
                worklist.push_back(Usr);
                continue;
            }
            return false;
        }
    }
    return true;
}

class UnsafeMem2Reg : public Mutation {
public:
    UnsafeMem2Reg() : Mutation(3) {
    }
    UnsafeMem2Reg(int decisions[]) : Mutation(3, decisions) {
    }

    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        std::vector<Function*> NonDeclFunctions;
        for (Function &F : *M) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                NonDeclFunctions.push_back(&F);
            }
        }

        if (NonDeclFunctions.empty()) {
            errs() << "No non-declaration functions found in the module.\n";
            return nullptr; // Return unchanged module instead of nullptr
        }
        
        // Select a random function
        Function* SelectedFunction = NonDeclFunctions[this->dm.make_decision(0, NonDeclFunctions.size() - 1)];

        // Find all basic blocks in the selected function
        std::vector<BasicBlock*> BasicBlocks;
        for (BasicBlock &BB : *SelectedFunction) {
            BasicBlocks.push_back(&BB);
        }

        if (BasicBlocks.empty()) {
            errs() << "No basic blocks found in the selected function.\n";
            return nullptr;
        }

        // Select a random basic block
        BasicBlock* SelectedBB = BasicBlocks[this->dm.make_decision(0, BasicBlocks.size() - 1)];
        std::vector<AllocaInst*> Allocas;
        for (Instruction &I : *SelectedBB) {
            if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)){
                Allocas.push_back(AI);
            }
        }
        
        if (Allocas.empty()){
            llvm::outs() << "No allocas found in the selected basic block.\n";
            return nullptr;
        }
        int AllocaIndexToPromote = dm.make_decision(0, Allocas.size() - 1);
        AllocaInst* AllocaToPromote = Allocas[AllocaIndexToPromote];
        
        // Create a dominator tree for the function
        DominatorTree DT;
        DT.recalculate(*SelectedFunction);
        
        // Promote the selected alloca to register
        std::vector<AllocaInst*> AllocasToPromote = {AllocaToPromote};
        if (simpleIsAllocaPromotable(AllocaToPromote)) {
            PromoteMemToReg(AllocasToPromote, DT);
        } else {
            llvm::outs() << "Alloca is not promotable\n";
            return nullptr;
        }
        return std::move(M);

    }
};