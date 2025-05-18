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
#include "mutation.h"
#include "utils.cpp"

using namespace llvm;

std::tuple<Instruction*, Instruction*, Instruction*> selectRandomInstructions(BasicBlock *BB, DecisionMaker &dm) {
    std::vector<Instruction*> Insts;
    for (Instruction &I : *BB) {
        Insts.push_back(&I);
    }

    if (Insts.size() < 3) {
        return std::make_tuple(nullptr, nullptr, nullptr);
    }

    int FirstIdx = dm.make_decision(0, Insts.size() - 2); // at least 1 before the end
    int LastIdx = dm.make_decision(FirstIdx + 1, Insts.size() - 1);

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
    int InsertPos = insertOptions[InsertIdx];

    Instruction *First = Insts[FirstIdx];
    Instruction *Last = Insts[LastIdx];
    Instruction *InsertBefore = Insts[InsertPos];

    return std::make_tuple(First, Last, InsertBefore);
}

class MoveBlockwise : public Mutation {
public:
    MoveBlockwise() : Mutation(4) {
    }
    MoveBlockwise(int decisions[]) : Mutation(4, decisions) {
    }
    
    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        // Find a random function that isn't a declaration
        std::vector<Function*> NonDeclFunctions;
        for (Function &F : *M) {
            if (!F.isDeclaration()) {
                NonDeclFunctions.push_back(&F);
            }
        }

        // Select a random function
        Function* SelectedFunction = NonDeclFunctions[this->dm.make_decision(0, NonDeclFunctions.size() - 1)];


        // Find all basic blocks in the selected function
        std::vector<BasicBlock*> BasicBlocks;
        for (BasicBlock &BB : *SelectedFunction) {
            BasicBlocks.push_back(&BB);
        }

        if (BasicBlocks.empty()) {
            return nullptr;
        }

        // Select a random basic block
        BasicBlock* SelectedBB = BasicBlocks[this->dm.make_decision(0, BasicBlocks.size() - 1)];


        std::tuple<Instruction*, Instruction*, Instruction*> Insts = selectRandomInstructions(SelectedBB, this->dm);
        
        if (std::get<0>(Insts) == nullptr || std::get<1>(Insts) == nullptr || std::get<2>(Insts) == nullptr) {
            return nullptr;
        }
        
        SelectedBB->splice(
            std::get<2>(Insts)->getIterator(), 
            SelectedBB,
            std::get<0>(Insts)->getIterator(), 
            ++std::get<1>(Insts)->getIterator()
        );
        
        return std::move(M);
    }
};
