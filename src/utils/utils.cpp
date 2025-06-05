#ifndef UTILS_CPP
#define UTILS_CPP

#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"

using namespace llvm;

bool isArithmeticInstruction(llvm::Instruction &I) {
    return llvm::isa<llvm::BinaryOperator>(&I) &&
           (I.getOpcode() == llvm::Instruction::Add ||
            I.getOpcode() == llvm::Instruction::Sub ||
            I.getOpcode() == llvm::Instruction::Mul ||
            I.getOpcode() == llvm::Instruction::UDiv ||
            I.getOpcode() == llvm::Instruction::SDiv);
}

#endif