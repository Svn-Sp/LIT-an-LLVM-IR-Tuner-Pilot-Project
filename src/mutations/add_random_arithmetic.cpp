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
#include "utils/is_optimizable.cpp"
#include "llvm/IR/Attributes.h"
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
    AddRandomArithmetic() : Mutation(5) {
    }
    AddRandomArithmetic(int decisions[]) : Mutation(5, decisions) {
    }

    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        // Find a random function that isn't a declaration
        std::vector<Function*> ValidFunctions;
        for (Function &F : *M) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                ValidFunctions.push_back(&F);
            }
        }
        if (ValidFunctions.empty()) {
            errs() << "No valid functions found in the module.\n";
            return nullptr; 
        }
        
        Function* SelectedFunction = ValidFunctions[this->dm.make_decision(0, ValidFunctions.size() - 1)];
        
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
        BasicBlock* SelectedBB1 = BasicBlocks[this->dm.make_decision(0, BasicBlocks.size() - 1)];
        
        // Find arithmetic instructions
        std::vector<Instruction*> ArithmeticInstructions;
        for (Instruction &I : *SelectedBB1) {
            if (isArithmeticInstruction(I)) {
                ArithmeticInstructions.push_back(&I);
            }
        }

        // Find instructions with numerical return type
        std::vector<Instruction*> NumericInstructions;
        for (Instruction* I : ArithmeticInstructions) {
            if (I->getType()->isIntegerTy() || I->getType()->isFloatingPointTy()) {
                NumericInstructions.push_back(I);
            }
        }
        
        if (NumericInstructions.size() < 2) {
            errs() << "Need at least 2 instructions with numerical return type, but only found " 
                << NumericInstructions.size() << ".\n";
            return nullptr;
        }
        
        int Idx1 = this->dm.make_decision(0, NumericInstructions.size() - 2);
        int Idx2 = this->dm.make_decision(Idx1 + 1, NumericInstructions.size() - 1);
        
        Instruction* Inst1 = NumericInstructions[Idx1];
        Instruction* Inst2 = NumericInstructions[Idx2];
    

        // Create a random arithmetic operation between Inst1 and Inst2
        Instruction::BinaryOps SelectedOp = ArithmeticOps[this->dm.make_decision(0, ArithmeticOps.size() - 1)];
        
        // Get the operand types
        Type* Ty1 = Inst1->getType();
        Type* Ty2 = Inst2->getType();
        
        Type* OpType = nullptr;
        if (Ty1->isIntegerTy() && Ty2->isIntegerTy()) {
            OpType = (cast<IntegerType>(Ty2)->getBitWidth() > cast<IntegerType>(Ty1)->getBitWidth()) ? Ty2 : Ty1;
        } else if (Ty1->isFloatingPointTy() && Ty2->isFloatingPointTy()) {
            OpType = (Ty1 == Ty2) ? Ty1 : nullptr;
        } else if (Ty1->isIntegerTy() && Ty2->isFloatingPointTy()) {
            // Allow int->float (int to float)
            OpType = Ty2;
        } else if (Ty1->isFloatingPointTy() && Ty2->isIntegerTy()) {
            OpType = nullptr;
        }
        if (!OpType) {
            errs() << "Unsupported type combination for arithmetic operation.\n";
            return nullptr;
        }
        
        // Create IRBuilder positioned after the first use
        IRBuilder<> Builder(Inst2->getNextNode());
        Value* Op1 = Inst1;
        Value* Op2 = Inst2;
        // Create casts if necessary
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
            llvm::outs() << "Op2: " << Op2->getName() << "will be casted to " << OpType<< "\n";
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
        std::string rand_suffix = std::to_string(rand());
        if (OpType->isFloatingPointTy()) {
            // For floating point types, use the FP operations
            switch (SelectedOp) {
                case Instruction::Add: NewInst = Builder.CreateFAdd(Op1, Op2, "random_add_" + rand_suffix); break;
                case Instruction::Sub: NewInst = Builder.CreateFSub(Op1, Op2, "random_sub_" + rand_suffix); break;
                case Instruction::Mul: NewInst = Builder.CreateFMul(Op1, Op2, "random_mul_" + rand_suffix); break;
                case Instruction::UDiv:
                case Instruction::SDiv: NewInst = Builder.CreateFDiv(Op1, Op2, "random_div_" + rand_suffix); break;
                default: break;
            }
        } else {
            // For integer types, use the regular operations
            switch (SelectedOp) {
                case Instruction::Add: NewInst = Builder.CreateAdd(Op1, Op2, "random_add_" + rand_suffix); break;
                case Instruction::Sub: NewInst = Builder.CreateSub(Op1, Op2, "random_sub_" + rand_suffix); break;
                case Instruction::Mul: NewInst = Builder.CreateMul(Op1, Op2, "random_mul_" + rand_suffix); break;
                case Instruction::UDiv: NewInst = Builder.CreateUDiv(Op1, Op2, "random_udiv_" + rand_suffix); break;
                case Instruction::SDiv: NewInst = Builder.CreateSDiv(Op1, Op2, "random_sdiv_" + rand_suffix); break;
                default: break;
            }
        }
        
        bool skip_first_use = false;
        for (auto &U : Inst2->uses()) {
            if (!skip_first_use) { //Skip first use 
                skip_first_use = true;
                continue;
            }
            U.set(NewInst);
        }
        return std::move(M);
    }
};

