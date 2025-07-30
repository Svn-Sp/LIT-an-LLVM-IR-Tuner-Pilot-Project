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
#include "llvm/IR/Verifier.h"
#include "utils/utils.cpp"
#include "utils/randomness_utils.cpp"
#include "utils/is_optimizable.cpp"
using namespace llvm;

class DeleteRandomInstruction : public Mutation {
public:
    DeleteRandomInstruction() : Mutation(3) {
    }
    DeleteRandomInstruction(int decisions[]) : Mutation(3, decisions) {
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
            return nullptr;
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

        // Collect all instructions in the selected basic block
        std::vector<Instruction*> Instructions;
        for (Instruction &I : *SelectedBB) {
            // Skip terminator instructions to avoid breaking the basic block
            if (!I.isTerminator()) {
                Instructions.push_back(&I);
            }
        }

        if (Instructions.empty()) {
            errs() << "No deletable instructions found in the selected basic block.\n";
            return nullptr;
        }

        // Select a random instruction to delete
        int instructionIndex = this->dm.make_decision(0, Instructions.size() - 1);
        Instruction* InstructionToDelete = Instructions[instructionIndex];

        // Store the instruction's parent basic block before deletion
        BasicBlock* parentBB = InstructionToDelete->getParent();
        if (!InstructionToDelete->use_empty()) {
            errs() << "Instruction still has uses, skipping deletion.\n";
            return nullptr;
        }

        // Delete the selected instruction
        InstructionToDelete->eraseFromParent();

        return std::move(M);
    }
};