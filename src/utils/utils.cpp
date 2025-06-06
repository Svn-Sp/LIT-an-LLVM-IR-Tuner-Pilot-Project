#ifndef UTILS_CPP
#define UTILS_CPP

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
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

std::unique_ptr<Module> countLLVMLines(std::unique_ptr<Module> M, int &lineCount) {
    lineCount = 0;
    
    // Count lines in each function
    for (const Function &F : *M) {
        // Count function declaration
        lineCount++;
        
        if (!F.isDeclaration()) {
            // Count each basic block and its instructions
            for (const BasicBlock &BB : F) {
                // Count basic block label
                lineCount++;
                
                // Count each instruction
                for (const Instruction &I : BB) {
                    lineCount++;
                }
            }
        }
    }
    
    return std::move(M);
}


#endif