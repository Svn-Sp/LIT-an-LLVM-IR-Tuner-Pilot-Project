#ifndef UTILS_CPP
#define UTILS_CPP

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

bool isArithmeticInstruction(Instruction &I) {
    return isa<BinaryOperator>(&I) &&
           (I.getOpcode() == Instruction::Add ||
            I.getOpcode() == Instruction::Sub ||
            I.getOpcode() == Instruction::Mul ||
            I.getOpcode() == Instruction::UDiv ||
            I.getOpcode() == Instruction::SDiv);
}

#endif