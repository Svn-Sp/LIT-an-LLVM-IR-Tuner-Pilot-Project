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
#include "mutations/mutation.h"
#include "utils/utils.cpp"
#include "utils/randomness_utils.cpp"
#include "utils/is_optimizable.cpp"
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
            if (!F.isDeclaration() && isOptimizable(&F)) {
                NonDeclFunctions.push_back(&F);
            }
        }

        if (NonDeclFunctions.empty()) {
            errs() << "No non-declaration functions found in the module.\n";
            return std::move(M); // Return unchanged module instead of nullptr
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
        Type* Type1 = SelectedRegister1->getType();
        Type* Type2 = SelectedRegister2->getType();
        
        // Handle type conversion and comparison
        Value* ComparisonResult = nullptr;
        
        // If types are identical, no conversion needed
        if (Type1 == Type2) {
            if (Type1->isIntegerTy() || Type1->isPointerTy()) {
                ComparisonResult = Builder.CreateICmpEQ(SelectedRegister1, SelectedRegister2, "regCompare");
            } else if (Type1->isFloatingPointTy()) {
                ComparisonResult = Builder.CreateFCmpOEQ(SelectedRegister1, SelectedRegister2, "regCompare");
            }
        } else {
            // Handle different types
            if (Type1->isIntegerTy() && Type2->isIntegerTy()) {
                // Both are integers - use integer comparison
                if (Type1->getIntegerBitWidth() != Type2->getIntegerBitWidth()) {
                    // Convert to the larger type
                    Type* LargerType = Type1->getIntegerBitWidth() > Type2->getIntegerBitWidth() ? Type1 : Type2;
                    if (Type1 != LargerType) {
                        SelectedRegister1 = Builder.CreateIntCast(SelectedRegister1, LargerType, true, "castedReg1");
                    }
                    if (Type2 != LargerType) {
                        SelectedRegister2 = Builder.CreateIntCast(SelectedRegister2, LargerType, true, "castedReg2");
                    }
                }
                ComparisonResult = Builder.CreateICmpEQ(SelectedRegister1, SelectedRegister2, "regCompare");
            } else if (Type1->isFloatingPointTy() && Type2->isFloatingPointTy()) {
                // Both are floating point - use floating point comparison
                Type* LargerType = Type1->getPrimitiveSizeInBits() > Type2->getPrimitiveSizeInBits() ? Type1 : Type2;
                if (Type1 != LargerType) {
                    SelectedRegister1 = Builder.CreateFPCast(SelectedRegister1, LargerType, "castedReg1");
                }
                if (Type2 != LargerType) {
                    SelectedRegister2 = Builder.CreateFPCast(SelectedRegister2, LargerType, "castedReg2");
                }
                ComparisonResult = Builder.CreateFCmpOEQ(SelectedRegister1, SelectedRegister2, "regCompare");
            } else if (Type1->isPointerTy() && Type2->isPointerTy()) {
                // Both are pointers - use pointer comparison
                ComparisonResult = Builder.CreateICmpEQ(SelectedRegister1, SelectedRegister2, "regCompare");
            } else if ((Type1->isIntegerTy() && Type2->isFloatingPointTy()) ||
                      (Type1->isFloatingPointTy() && Type2->isIntegerTy())) {
                // One is integer, one is floating point - convert both to floating point
                Type* FloatType = Type1->isFloatingPointTy() ? Type1 : Type2;
                if (Type1 != FloatType) {
                    SelectedRegister1 = Builder.CreateSIToFP(SelectedRegister1, FloatType, "castedReg1");
                }
                if (Type2 != FloatType) {
                    SelectedRegister2 = Builder.CreateSIToFP(SelectedRegister2, FloatType, "castedReg2");
                }
                ComparisonResult = Builder.CreateFCmpOEQ(SelectedRegister1, SelectedRegister2, "regCompare");
            } else {
                errs() << "Cannot compare registers of incompatible types: " 
                      << *Type1 << " and " << *Type2 << "\n";
                return nullptr;
            }
        }

        if (!ComparisonResult) {
            errs() << "Failed to create comparison between registers.\n";
            return nullptr;
        }

        // Create a new basic block that will be jumped to if the registers are equal
        std::uniform_int_distribution<> dis(0, 1000000);
        Twine Name = Twine("newblock") + Twine(dis(gen));
        BasicBlock* NewBlock = BasicBlock::Create(M->getContext(), Name, SelectedFunction);
        // llvm::outs()<<"NewBlock: "<<NewBlock<<"\n";
        // Get the original successor of the terminator
        BasicBlock* OriginalSuccessor = nullptr;
        if (BranchInst* BI = dyn_cast<BranchInst>(Terminator)) {
            OriginalSuccessor = BI->getSuccessor(0);
        } else if (SwitchInst* SI = dyn_cast<SwitchInst>(Terminator)) {
            // For switches, use the default destination
            OriginalSuccessor = SI->getDefaultDest();
        } else {
            // For other terminators, we can't easily determine a successor
            errs() << "Unsupported terminator type for this mutation.\n";
            return nullptr;
        }
        
        IRBuilder<> NewBlockBuilder(NewBlock);  

        // Set up branching in NewBlock to the original successor
        NewBlockBuilder.CreateBr(OriginalSuccessor);
        Builder.CreateCondBr(ComparisonResult, NewBlock, OriginalSuccessor);
        // Remove the original terminator from the selected basic block if it exists
        if (SelectedBB->getTerminator()) {
            SelectedBB->getTerminator()->eraseFromParent();
        }
        // llvm::outs()<<"Mutation done"<<"\n";
        return std::move(M);
    }
};