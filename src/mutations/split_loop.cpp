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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include <vector>
#include <fstream>
#include <random>
#include "mutations/mutation.h"
#include "utils/is_optimizable.cpp"
#include "utils/utils.cpp"

using namespace llvm;

class SplitLoop : public Mutation {
public:
    SplitLoop() : Mutation(5) {
    }
    SplitLoop(int decisions[]) : Mutation(5, decisions) {
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
            return nullptr;
        }

        Function* SelectedFunction = NonDeclFunctions[this->dm.make_decision(0, NonDeclFunctions.size() - 1)];
        
        // Create LoopInfo and DominatorTree for loop analysis
        DominatorTree DT(*SelectedFunction);
        LoopInfo LI;
        LI.analyze(DT);
        
        std::vector<Loop*> Loops = LI.getTopLevelLoops();
        if (Loops.empty()) {
            errs() << "No loops found in the selected function.\n";
            return nullptr;
        }
        
        Loop* SelectedLoop = Loops[this->dm.make_decision(0, Loops.size() - 1)];
        
        // Get the loop header block
        BasicBlock* Header = SelectedLoop->getHeader();
        if (!Header) {
            errs() << "No header found for selected loop.\n";
            return nullptr;
        }
        
        // Find the loop induction variable and bounds
        PHINode* IndVar = nullptr;
        Value* StartValue = nullptr;
        Value* EndValue = nullptr;
        Value* StepValue = nullptr;
        
        // Look for canonical loop structure: for (i = start; i < end; i += step)
        for (Instruction& I : *Header) {
            if (PHINode* PN = dyn_cast<PHINode>(&I)) {
                // Check if this is a loop induction variable
                if (PN->getNumIncomingValues() == 2) {
                    // One incoming value should be the initial value, other should be the increment
                    Value* InitVal = nullptr;
                    Value* IncVal = nullptr;
                    
                    for (unsigned i = 0; i < PN->getNumIncomingValues(); ++i) {
                        BasicBlock* IncomingBB = PN->getIncomingBlock(i);
                        if (IncomingBB == Header) {
                            // This is the increment value
                            IncVal = PN->getIncomingValue(i);
                        } else {
                            // This is the initial value
                            InitVal = PN->getIncomingValue(i);
                        }
                    }
                    
                    if (InitVal && IncVal) {
                        IndVar = PN;
                        StartValue = InitVal;
                        
                        // Find the loop condition to get end value
                        for (BasicBlock* Succ : successors(Header)) {
                            if (SelectedLoop->contains(Succ)) {
                                // This is the loop body, find the condition
                                for (Instruction& SuccI : *Succ) {
                                    if (ICmpInst* ICmp = dyn_cast<ICmpInst>(&SuccI)) {
                                        if (ICmp->getPredicate() == ICmpInst::ICMP_SLT || 
                                            ICmpInst::ICMP_ULT) {
                                            // Found the loop condition
                                            if (ICmp->getOperand(0) == IndVar) {
                                                EndValue = ICmp->getOperand(1);
                                                StepValue = IncVal;
                                                break;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        
                        if (EndValue && StepValue) {
                            break;
                        }
                    }
                }
            }
        }
        
        if (!IndVar || !StartValue || !EndValue || !StepValue) {
            errs() << "Could not identify canonical loop structure for splitting.\n";
            return nullptr;
        }
        
        // Check if we can split the loop (need constant bounds and step)
        ConstantInt* StartConst = dyn_cast<ConstantInt>(StartValue);
        ConstantInt* EndConst = dyn_cast<ConstantInt>(EndValue);
        ConstantInt* StepConst = dyn_cast<ConstantInt>(StepValue);
        
        if (!StartConst || !EndConst || !StepConst) {
            errs() << "Loop bounds or step are not constant, cannot split.\n";
            return nullptr;
        }
        
        int64_t Start = StartConst->getSExtValue();
        int64_t End = EndConst->getSExtValue();
        int64_t Step = StepConst->getSExtValue();
        
        if (Step <= 0) {
            errs() << "Loop step must be positive for splitting.\n";
            return nullptr;
        }
        
        // Calculate the midpoint
        int64_t TotalIterations = (End - Start) / Step;
        if (TotalIterations < 2) {
            errs() << "Loop has too few iterations to split meaningfully.\n";
            return nullptr;
        }
        
        int64_t MidPoint = Start + (TotalIterations / 2) * Step;
        
        // Create the split loops
        IRBuilder<> Builder(Header);
        
        // Create new loop variables
        Type* IndVarType = IndVar->getType();
        Value* NewIndVar1 = Builder.CreateAlloca(IndVarType, nullptr, "split_loop_var1");
        Value* NewIndVar2 = Builder.CreateAlloca(IndVarType, nullptr, "split_loop_var2");
        
        // Initialize the first loop variable
        Builder.CreateStore(StartValue, NewIndVar1);
        
        // Create the first loop (from start to midpoint)
        BasicBlock* Loop1Header = BasicBlock::Create(M->getContext(), "split_loop1_header", SelectedFunction);
        BasicBlock* Loop1Body = BasicBlock::Create(M->getContext(), "split_loop1_body", SelectedFunction);
        BasicBlock* Loop1Exit = BasicBlock::Create(M->getContext(), "split_loop1_exit", SelectedFunction);
        
        // Initialize the second loop variable
        Builder.CreateStore(ConstantInt::get(IndVarType, MidPoint), NewIndVar2);
        
        // Create the second loop (from midpoint to end)
        BasicBlock* Loop2Header = BasicBlock::Create(M->getContext(), "split_loop2_header", SelectedFunction);
        BasicBlock* Loop2Body = BasicBlock::Create(M->getContext(), "split_loop2_body", SelectedFunction);
        BasicBlock* Loop2Exit = BasicBlock::Create(M->getContext(), "split_loop2_exit", SelectedFunction);
        
        // Set up the first loop
        Builder.SetInsertPoint(Loop1Header);
        Value* CurrentVal1 = Builder.CreateLoad(IndVarType, NewIndVar1, "current1");
        Value* Cond1 = Builder.CreateICmpSLT(CurrentVal1, ConstantInt::get(IndVarType, MidPoint));
        Builder.CreateCondBr(Cond1, Loop1Body, Loop1Exit);
        
        // Set up the second loop
        Builder.SetInsertPoint(Loop2Header);
        Value* CurrentVal2 = Builder.CreateLoad(IndVarType, NewIndVar2, "current2");
        Value* Cond2 = Builder.CreateICmpSLT(CurrentVal2, EndValue);
        Builder.CreateCondBr(Cond2, Loop2Body, Loop2Exit);
        
        // Copy the original loop body to both new loops
        // This is a simplified approach - in practice, you'd need to clone the entire loop body
        // and update all references to the original induction variable
        
        // For now, we'll create a simple increment in each loop body
        Builder.SetInsertPoint(Loop1Body);
        Value* NewVal1 = Builder.CreateAdd(CurrentVal1, StepValue);
        Builder.CreateStore(NewVal1, NewIndVar1);
        Builder.CreateBr(Loop1Header);
        
        Builder.SetInsertPoint(Loop2Body);
        Value* NewVal2 = Builder.CreateAdd(CurrentVal2, StepValue);
        Builder.CreateStore(NewVal2, NewIndVar2);
        Builder.CreateBr(Loop2Header);
        
        // Connect the loops
        Builder.SetInsertPoint(Loop1Exit);
        Builder.CreateBr(Loop2Header);
        
        Builder.SetInsertPoint(Loop2Exit);
        
        // Replace the original loop with our split loops
        // This is a simplified replacement - in practice, you'd need to:
        // 1. Find all predecessors of the original loop header
        // 2. Redirect them to the first new loop header
        // 3. Find all successors of the original loop exit
        // 4. Redirect the second loop exit to those successors
        
        // For demonstration, we'll just add the new loops after the original function
        // In practice, you'd replace the original loop structure
        
        return std::move(M);
    }
};