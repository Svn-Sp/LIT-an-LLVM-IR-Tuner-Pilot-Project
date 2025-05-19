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

std::vector<Instruction::BinaryOps> ArithmeticOps = {
    Instruction::Add,
    Instruction::Sub,
    Instruction::Mul,
    Instruction::UDiv,
    Instruction::SDiv
};


class AddRandomArithmetic : public Mutation {
public:
    AddRandomArithmetic() : Mutation(6) {
    }
    AddRandomArithmetic(int decisions[]) : Mutation(6, decisions) {
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
        llvm::outs() << "Decision maker index: " << this->dm.current_decision << "\n";
        llvm::outs() << "Next decision: " << this->dm.decisions[this->dm.current_decision] << "\n";
        llvm::outs() << "NonDeclFunctions.size(): " << NonDeclFunctions.size() << "\n";
        Function* SelectedFunction = NonDeclFunctions[this->dm.make_decision(0, NonDeclFunctions.size() - 1)];
        llvm::outs() << "Selected function: " << SelectedFunction->getName() << "\n";
        
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
        
        // Find arithmetic instructions
        std::vector<Instruction*> ArithmeticInstructions;
        for (Instruction &I : *SelectedBB) {
            if (isArithmeticInstruction(I)) {
                ArithmeticInstructions.push_back(&I);
            }
        }

        // Find instructions with numerical return type
        std::vector<Instruction*> NumericInstructions;
        for (Instruction &I : *SelectedBB) {
            if (I.getType()->isIntegerTy() || I.getType()->isFloatingPointTy()) {
                NumericInstructions.push_back(&I);
            }
        }
        
        if (NumericInstructions.size() < 2) {
            errs() << "Need at least 2 instructions with numerical return type, but only found " 
                << NumericInstructions.size() << ".\n";
            return nullptr;
        }
        
        int FirstIdx = this->dm.make_decision(0, NumericInstructions.size() - 2);
        int SecondIdx = this->dm.make_decision(FirstIdx + 1, NumericInstructions.size() - 1);
        
        Instruction* Inst1 = NumericInstructions[FirstIdx];
        Instruction* Inst2 = NumericInstructions[SecondIdx];
    

        // Create a random arithmetic operation between Inst1 and Inst2
        Instruction::BinaryOps SelectedOp = ArithmeticOps[this->dm.make_decision(0, ArithmeticOps.size() - 1)];
        
        // Get the operand types
        Type* Ty1 = Inst1->getType();
        Type* Ty2 = Inst2->getType();
        
        // Choose the type for the operation (prefer the wider type)
        Type* OpType = Ty1;
        if (Ty1->isIntegerTy() && Ty2->isIntegerTy()) {
            if (cast<IntegerType>(Ty2)->getBitWidth() > cast<IntegerType>(Ty1)->getBitWidth()) {
                OpType = Ty2;
            }
        } else if (Ty1->isFloatingPointTy() && Ty2->isFloatingPointTy()) {
            // For floating point, prefer double over float
            if (Ty2->isDoubleTy() && !Ty1->isDoubleTy()) {
                OpType = Ty2;
            }
        } else if (Ty1->isIntegerTy() && Ty2->isFloatingPointTy()) {
            // If mixing integer and float, use the float type
            OpType = Ty2;
        } else if (Ty1->isFloatingPointTy() && Ty2->isIntegerTy()) {
            OpType = Ty1;
        }
        
        // Create casts if necessary
        Value* Op1 = Inst1;
        Value* Op2 = Inst2;
        IRBuilder<> Builder(Inst2->getNextNode());
        
        if (Ty1 != OpType) {
            if (Ty1->isIntegerTy() && OpType->isIntegerTy()) {
                Op1 = Builder.CreateIntCast(Inst1, OpType, true, "cast_op1");
            } else if (Ty1->isIntegerTy() && OpType->isFloatingPointTy()) {
                Op1 = Builder.CreateSIToFP(Inst1, OpType, "cast_op1");
            } else if (Ty1->isFloatingPointTy() && OpType->isFloatingPointTy()) {
                Op1 = Builder.CreateFPCast(Inst1, OpType, "cast_op1");
            }
        }
        
        if (Ty2 != OpType) {
            if (Ty2->isIntegerTy() && OpType->isIntegerTy()) {
                Op2 = Builder.CreateIntCast(Inst2, OpType, true, "cast_op2");
            } else if (Ty2->isIntegerTy() && OpType->isFloatingPointTy()) {
                Op2 = Builder.CreateSIToFP(Inst2, OpType, "cast_op2");
            } else if (Ty2->isFloatingPointTy() && OpType->isFloatingPointTy()) {
                Op2 = Builder.CreateFPCast(Inst2, OpType, "cast_op2");
            }
        }
        
        // Create the new instruction
        Value* NewInst = nullptr;
        if (OpType->isFloatingPointTy()) {
            // For floating point types, use the FP operations
            switch (SelectedOp) {
                case Instruction::Add: NewInst = Builder.CreateFAdd(Op1, Op2, "random_add"); break;
                case Instruction::Sub: NewInst = Builder.CreateFSub(Op1, Op2, "random_sub"); break;
                case Instruction::Mul: NewInst = Builder.CreateFMul(Op1, Op2, "random_mul"); break;
                case Instruction::UDiv:
                case Instruction::SDiv: NewInst = Builder.CreateFDiv(Op1, Op2, "random_div"); break;
                default: break;
            }
        } else {
            // For integer types, use the regular operations
            switch (SelectedOp) {
                case Instruction::Add: NewInst = Builder.CreateAdd(Op1, Op2, "random_add"); break;
                case Instruction::Sub: NewInst = Builder.CreateSub(Op1, Op2, "random_sub"); break;
                case Instruction::Mul: NewInst = Builder.CreateMul(Op1, Op2, "random_mul"); break;
                case Instruction::UDiv: NewInst = Builder.CreateUDiv(Op1, Op2, "random_udiv"); break;
                case Instruction::SDiv: NewInst = Builder.CreateSDiv(Op1, Op2, "random_sdiv"); break;
                default: break;
            }
        }
        

        // Randomly select one of the original instructions to replace
        Instruction* targetToReplace = (this->dm.make_decision(0, 1) == 0) ? Inst1 : Inst2;
        
        if (NewInst) {
            // Replace all uses of the selected target with our new instruction
            targetToReplace->replaceAllUsesWith(NewInst);
        }
        
        return std::move(M);
    }
};
