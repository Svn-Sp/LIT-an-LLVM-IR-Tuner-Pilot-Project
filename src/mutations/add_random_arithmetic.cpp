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
#include "llvm/IR/Dominators.h"
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

static bool isNumericScalarValue(const Value *V) {
    Type *T = V->getType();
    return !T->isVoidTy() && !T->isVectorTy() &&
           (T->isIntegerTy() || T->isFloatingPointTy());
}

/// Values visible at the program point immediately before \p InsertBefore (SSA + dominance).
static void collectValuesInScope(Function *F, const DominatorTree &DT,
                                 Instruction *InsertBefore,
                                 std::vector<Value *> &Out) {
    for (Argument &A : F->args()) {
        if (isNumericScalarValue(&A))
            Out.push_back(&A);
    }
    for (BasicBlock &BB : *F) {
        for (Instruction &I : BB) {
            if (!isNumericScalarValue(&I))
                continue;
            if (&I == InsertBefore)
                continue;
            if (!DT.dominates(&I, InsertBefore))
                continue;
            Out.push_back(&I);
        }
    }
}

class AddRandomArithmetic : public Mutation {
public:
    AddRandomArithmetic() : Mutation() {
    }
    AddRandomArithmetic(std::vector<int> decisions) : Mutation(std::move(decisions)) {
    }

    std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override {
        std::vector<Function *> ValidFunctions;
        for (Function &F : *M) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                ValidFunctions.push_back(&F);
            }
        }
        if (ValidFunctions.empty()) {
            errs() << "No valid functions found in the module.\n";
            return nullptr;
        }

        Function *SelectedFunction =
            ValidFunctions[this->dm.make_decision(0, ValidFunctions.size() - 1)];

        std::vector<BasicBlock *> BasicBlocks;
        for (BasicBlock &BB : *SelectedFunction) {
            BasicBlocks.push_back(&BB);
        }

        if (BasicBlocks.empty()) {
            errs() << "No basic blocks found in the selected function.\n";
            return nullptr;
        }

        BasicBlock *SelectedBB =
            BasicBlocks[this->dm.make_decision(0, BasicBlocks.size() - 1)];

        // Insert before a non-phi instruction so the new binop stays after any PHIs.
        std::vector<Instruction *> InsertionPoints;
        for (Instruction &I : *SelectedBB) {
            if (!isa<PHINode>(&I))
                InsertionPoints.push_back(&I);
        }

        if (InsertionPoints.empty()) {
            errs() << "No non-phi insertion point in the selected basic block.\n";
            return nullptr;
        }

        Instruction *InsertBefore = InsertionPoints[this->dm.make_decision(
            0, InsertionPoints.size() - 1)];

        DominatorTree DT;
        DT.recalculate(*SelectedFunction);

        std::vector<Value *> ScopedValues;
        collectValuesInScope(SelectedFunction, DT, InsertBefore, ScopedValues);

        if (ScopedValues.size() < 2) {
            errs() << "Need at least 2 in-scope numeric values, found "
                   << ScopedValues.size() << ".\n";
            return nullptr;
        }

        int Idx1 = this->dm.make_decision(0, ScopedValues.size() - 2);
        int Idx2 = this->dm.make_decision(Idx1 + 1, ScopedValues.size() - 1);

        Value *Val1 = ScopedValues[Idx1];
        Value *Val2 = ScopedValues[Idx2];

        Instruction::BinaryOps SelectedOp =
            ArithmeticOps[this->dm.make_decision(0, ArithmeticOps.size() - 1)];

        Type *Ty1 = Val1->getType();
        Type *Ty2 = Val2->getType();

        Type *OpType = nullptr;
        if (Ty1->isIntegerTy() && Ty2->isIntegerTy()) {
            OpType = (cast<IntegerType>(Ty2)->getBitWidth() >
                      cast<IntegerType>(Ty1)->getBitWidth())
                         ? Ty2
                         : Ty1;
        } else if (Ty1->isFloatingPointTy() && Ty2->isFloatingPointTy()) {
            OpType = (Ty1 == Ty2) ? Ty1 : nullptr;
        } else if (Ty1->isIntegerTy() && Ty2->isFloatingPointTy()) {
            OpType = Ty2;
        } else if (Ty1->isFloatingPointTy() && Ty2->isIntegerTy()) {
            OpType = nullptr;
        }
        if (!OpType) {
            errs() << "Unsupported type combination for arithmetic operation.\n";
            return nullptr;
        }

        IRBuilder<> Builder(InsertBefore);
        Value *Op1 = Val1;
        Value *Op2 = Val2;

        if (Ty1 != OpType) {
            if (Ty1->isIntegerTy() && OpType->isIntegerTy()) {
                Op1 = Builder.CreateIntCast(Val1, OpType, true, "cast_op1");
            } else if (Ty1->isIntegerTy() && OpType->isFloatingPointTy()) {
                Op1 = Builder.CreateSIToFP(Val1, OpType, "cast_op1");
            } else if (Ty1->isFloatingPointTy() && OpType->isFloatingPointTy()) {
                Op1 = Builder.CreateFPCast(Val1, OpType, "cast_op1");
            }
        }

        if (Ty2 != OpType) {
            if (Ty2->isIntegerTy() && OpType->isIntegerTy()) {
                Op2 = Builder.CreateIntCast(Val2, OpType, true, "cast_op2");
            } else if (Ty2->isIntegerTy() && OpType->isFloatingPointTy()) {
                Op2 = Builder.CreateSIToFP(Val2, OpType, "cast_op2");
            } else if (Ty2->isFloatingPointTy() && OpType->isFloatingPointTy()) {
                Op2 = Builder.CreateFPCast(Val2, OpType, "cast_op2");
            }
        }

        Value *NewInst = nullptr;
        std::string rand_suffix = std::to_string(rand());
        if (OpType->isFloatingPointTy()) {
            switch (SelectedOp) {
            case Instruction::Add:
                NewInst = Builder.CreateFAdd(Op1, Op2, "random_add_" + rand_suffix);
                break;
            case Instruction::Sub:
                NewInst = Builder.CreateFSub(Op1, Op2, "random_sub_" + rand_suffix);
                break;
            case Instruction::Mul:
                NewInst = Builder.CreateFMul(Op1, Op2, "random_mul_" + rand_suffix);
                break;
            case Instruction::UDiv:
            case Instruction::SDiv:
                NewInst = Builder.CreateFDiv(Op1, Op2, "random_div_" + rand_suffix);
                break;
            default:
                break;
            }
        } else {
            switch (SelectedOp) {
            case Instruction::Add:
                NewInst = Builder.CreateAdd(Op1, Op2, "random_add_" + rand_suffix);
                break;
            case Instruction::Sub:
                NewInst = Builder.CreateSub(Op1, Op2, "random_sub_" + rand_suffix);
                break;
            case Instruction::Mul:
                NewInst = Builder.CreateMul(Op1, Op2, "random_mul_" + rand_suffix);
                break;
            case Instruction::UDiv:
                NewInst = Builder.CreateUDiv(Op1, Op2, "random_udiv_" + rand_suffix);
                break;
            case Instruction::SDiv:
                NewInst = Builder.CreateSDiv(Op1, Op2, "random_sdiv_" + rand_suffix);
                break;
            default:
                break;
            }
        }

        if (!NewInst) {
            errs() << "Failed to create arithmetic instruction.\n";
            return nullptr;
        }

        // Same idea as before: steer some uses of the second operand toward the new value.
        if (Instruction *ReplaceTarget = dyn_cast<Instruction>(Val2)) {
            bool skip_first_use = false;
            for (Use &U : ReplaceTarget->uses()) {
                if (!skip_first_use) {
                    skip_first_use = true;
                    continue;
                }
                U.set(NewInst);
            }
        }

        return std::move(M);
    }
};
