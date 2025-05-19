#pragma once
#include <vector>
#include <fstream>
#include <random>
#include <stdio.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "mutation.h"
#include "utils.cpp"
using namespace llvm;

class AddNewCond : public Mutation {
public:
    AddNewCond() : Mutation(4) {
    }
    AddNewCond(int decisions[]) : Mutation(4, decisions) {
    }

    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        std::vector<Function*> NonDeclFunctions;
        for (Function &F : *M) {
            if (!F.isDeclaration()) {
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
        // Find the terminator instruction of the selected basic block
        Instruction* Terminator = SelectedBB->getTerminator();
        if (!Terminator) {
            errs() << "Selected basic block has no terminator instruction.\n";
            return nullptr;
        }
        
        // Position the IRBuilder right before the terminator instruction
        IRBuilder<> Builder(Terminator);

        //Select all used registers from the function
        std::vector<Value*> UsedRegisters;
        for (BasicBlock &BB : *SelectedFunction) {
            for (Instruction &I : BB) {
                for (User *U : I.users()) {
                    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                        for (Value *Operand : Inst->operands()) {
                            if (Instruction *OperandInst = dyn_cast<Instruction>(Operand)) {
                                if (std::find(UsedRegisters.begin(), UsedRegisters.end(), OperandInst) == UsedRegisters.end()) {
                                    UsedRegisters.push_back(OperandInst);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Ensure we have at least two registers to compare
        if (UsedRegisters.size() < 2) {
            errs() << "Not enough registers to perform the mutation.\n";
            return std::move(M);
        }
        
        // Select two random registers
        Value* SelectedRegister1 = UsedRegisters[this->dm.make_decision(0, UsedRegisters.size() - 1)];
        Value* SelectedRegister2 = UsedRegisters[this->dm.make_decision(0, UsedRegisters.size() - 1)];
        
        // Ensure both registers have the same type for comparison
        if (SelectedRegister1->getType() != SelectedRegister2->getType()) {
            // Try to cast if possible, or just return without mutation if not possible
            if (CastInst::isBitCastable(SelectedRegister1->getType(), SelectedRegister2->getType())) {
                SelectedRegister2 = Builder.CreateCast(
                    CastInst::getCastOpcode(SelectedRegister2, false, SelectedRegister1->getType(), false),
                    SelectedRegister2, SelectedRegister1->getType(), "castedReg");
            } else if (CastInst::isBitCastable(SelectedRegister2->getType(), SelectedRegister1->getType())) {
                SelectedRegister1 = Builder.CreateCast(
                    CastInst::getCastOpcode(SelectedRegister1, false, SelectedRegister2->getType(), false),
                    SelectedRegister1, SelectedRegister2->getType(), "castedReg");
            } else {
                errs() << "Cannot compare registers of incompatible types.\n";
                return nullptr;
            }
        }
        
        // Compare the two registers for equality
        Value* ComparisonResult = Builder.CreateICmpEQ(SelectedRegister1, SelectedRegister2, "regCompare");
        
        // Create a new basic block B1 that will be jumped to if the registers are equal
        BasicBlock* B1 = BasicBlock::Create(M->getContext(), "newblock_B1", SelectedFunction, nullptr);
        
        // Get the original successor of the terminator
        BasicBlock* OriginalSuccessor = nullptr;
        if (BranchInst* BI = dyn_cast<BranchInst>(Terminator)) {
            if (BI->isUnconditional()) {
                OriginalSuccessor = BI->getSuccessor(0);
            } else {
                // For conditional branches, use the true successor
                OriginalSuccessor = BI->getSuccessor(0);
            }
        } else if (SwitchInst* SI = dyn_cast<SwitchInst>(Terminator)) {
            // For switches, use the default destination
            OriginalSuccessor = SI->getDefaultDest();
        } else {
            // For other terminators, we can't easily determine a successor
            errs() << "Unsupported terminator type for this mutation.\n";
            return nullptr;
        }
        
        IRBuilder<> B1Builder(B1);  
        // Set up branching in B1 to the original successor
        B1Builder.CreateBr(OriginalSuccessor);
        Builder.CreateCondBr(ComparisonResult, B1, OriginalSuccessor);

        return std::move(M);
    }
};