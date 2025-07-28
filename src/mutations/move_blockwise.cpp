#pragma once
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include <vector>
#include <fstream>
#include <random>
#include "mutations/mutation.h"
#include "utils/is_optimizable.cpp"
#include "utils/utils.cpp"

using namespace llvm;

std::tuple<Instruction*, Instruction*, Instruction*> selectRandomInstructions(BasicBlock *BB, DecisionMaker &dm) {
    // Safety check for nullptr
    if (!BB) {
        return std::make_tuple(nullptr, nullptr, nullptr);
    }
    
    std::vector<Instruction*> Insts;
    try {
        for (Instruction &I : *BB) {
            Insts.push_back(&I);
        }
    } catch (...) {
        // If iteration fails, return nullptrs
        return std::make_tuple(nullptr, nullptr, nullptr);
    }

    if (Insts.size() < 3) {
        return std::make_tuple(nullptr, nullptr, nullptr);
    }

    int FirstIdx = dm.make_decision(0, Insts.size() - 2); // at least 1 before the end
    // Bounds check
    if (FirstIdx < 0 || FirstIdx >= static_cast<int>(Insts.size() - 1)) {
        FirstIdx = 0;
    }
    
    int LastIdx = dm.make_decision(FirstIdx + 1, Insts.size() - 1);
    // Bounds check
    if (LastIdx <= FirstIdx || LastIdx >= static_cast<int>(Insts.size())) {
        LastIdx = std::min(FirstIdx + 1, static_cast<int>(Insts.size() - 1));
    }

    std::vector<int> insertOptions;
    for (int i = 0; i < (int)Insts.size(); ++i) {
        if (i < FirstIdx || i > LastIdx) {
            insertOptions.push_back(i);
        }
    }

    if (insertOptions.empty()) {
        return std::make_tuple(nullptr, nullptr, nullptr);
    }

    int InsertIdx = dm.make_decision(0, insertOptions.size() - 1);
    // Bounds check
    if (InsertIdx < 0 || InsertIdx >= static_cast<int>(insertOptions.size())) {
        InsertIdx = 0;
    }
    
    int InsertPos = insertOptions[InsertIdx];

    Instruction *First = Insts[FirstIdx];
    Instruction *Last = Insts[LastIdx];
    Instruction *InsertBefore = Insts[InsertPos];
    
    // Final validity check
    if (!First || !Last || !InsertBefore) {
        return std::make_tuple(nullptr, nullptr, nullptr);
    }

    return std::make_tuple(First, Last, InsertBefore);
}

class MoveBlockwise : public Mutation {
public:
    MoveBlockwise() : Mutation(5) {
    }
    MoveBlockwise(int decisions[]) : Mutation(5, decisions) {
    }
    
    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        // Find a random function that isn't a declaration
        std::vector<Function*> NonDeclFunctions;
        for (Function &F : *M) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                NonDeclFunctions.push_back(&F);
            }
        }

        // Safety check: ensure we have at least one function
        if (NonDeclFunctions.empty()) {
            errs() << "No non-declaration functions found in the module.\n";
            return nullptr; // Return unchanged module instead of nullptr
        }

        // Make sure decision index is in bounds
        int funcIndex = this->dm.make_decision(0, NonDeclFunctions.size() - 1);
        if (funcIndex < 0 || funcIndex >= static_cast<int>(NonDeclFunctions.size())) {
            // If the decision is out of bounds, default to the first function
            funcIndex = 0;
        }

        // Select a random function
        Function* SelectedFunction = NonDeclFunctions[funcIndex];

        // Safety check: ensure SelectedFunction is valid
        if (!SelectedFunction) {
            return nullptr; // Return unchanged module
        }

        // Find all basic blocks in the selected function
        std::vector<BasicBlock*> BasicBlocks;
        
        // Safely iterate through basic blocks with validity check
        bool validFunction = true;
        try {
            for (BasicBlock &BB : *SelectedFunction) {
                BasicBlocks.push_back(&BB);
            }
        } catch (...) {
            // If any exception occurs during iteration, consider it invalid
            validFunction = false;
        }

        if (!validFunction || BasicBlocks.empty()) {
            return nullptr; // Return unchanged module
        }

        // Make sure decision index is in bounds
        int bbIndex = this->dm.make_decision(0, BasicBlocks.size() - 1);
        if (bbIndex < 0 || bbIndex >= static_cast<int>(BasicBlocks.size())) {
            // If the decision is out of bounds, default to the first basic block
            bbIndex = 0;
        }

        // Select a random basic block
        BasicBlock* SelectedBB = BasicBlocks[bbIndex];

        // Safety check: ensure SelectedBB is valid
        if (!SelectedBB) {
            return nullptr; // Return unchanged module
        }

        std::tuple<Instruction*, Instruction*, Instruction*> Insts = selectRandomInstructions(SelectedBB, this->dm);
        
        if (std::get<0>(Insts) == nullptr || std::get<1>(Insts) == nullptr || std::get<2>(Insts) == nullptr) {
            return nullptr;
        }
        
        // Safety check: ensure instructions are valid before splicing
        try {
            SelectedBB->splice(
                std::get<2>(Insts)->getIterator(), 
                SelectedBB,
                std::get<0>(Insts)->getIterator(), 
                ++std::get<1>(Insts)->getIterator()
            );
        } catch (...) {
            return nullptr;
        }
        
        return std::move(M);
    }
};
