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
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "mutations/mutation.h"
#include <vector>
#include "utils/utils.cpp"
#include <fstream>
#include <random>

using namespace llvm;

class ReplaceArithmetic : public Mutation {
    public:
        ReplaceArithmetic() : Mutation(3) {
        }
        ReplaceArithmetic(int decisions[]) : Mutation(3, decisions) {
        }
        std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) override{
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
                errs() << "No basic blocks found in the selected function.\n";
                return nullptr;
            }

            // Select a random basic block
            BasicBlock* SelectedBB = BasicBlocks[this->dm.make_decision(0, BasicBlocks.size() - 1)];


            //Find random arithmetic instruction
            std::vector<Instruction*> ArithmeticInstructions;
            for (Instruction &I : *SelectedBB) {
                if (isArithmeticInstruction(I)) {   
                    ArithmeticInstructions.push_back(&I);
                }
            }

            if (ArithmeticInstructions.empty()) {
                errs() << "No arithmetic instructions found in the selected basic block.\n";
                return nullptr;
            }

            Instruction* SelectedArithmetic = ArithmeticInstructions[this->dm.make_decision(0, ArithmeticInstructions.size() - 1)];

            
            //Replace the arithmetic operation with the opposite arithmetic operation
            // SelectedArithmetic->setOperand(0, SelectedArithmetic->getOperand(1));
            llvm::IRBuilder<> NewInstrBuilder(SelectedArithmetic);
            llvm::Value *NewInst;
            if (SelectedArithmetic->getOpcode() == Instruction::Add) {
                NewInst = NewInstrBuilder.CreateSub(SelectedArithmetic->getOperand(0), SelectedArithmetic->getOperand(1));
            } else if (SelectedArithmetic->getOpcode() == Instruction::Sub) {
                NewInst = NewInstrBuilder.CreateAdd(SelectedArithmetic->getOperand(0), SelectedArithmetic->getOperand(1));
            } else {
                NewInst = NewInstrBuilder.CreateMul(SelectedArithmetic->getOperand(0), SelectedArithmetic->getOperand(1));
            }
            SelectedArithmetic->replaceAllUsesWith(NewInst);
            SelectedArithmetic->eraseFromParent();
            return std::move(M);
        };
};


// int main(int argc, char** argv) {

//     ReplaceArithmetic ra(3);
//     ra.run(argc, argv);

//     return 0;
// }