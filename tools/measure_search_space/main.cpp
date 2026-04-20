#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>

// LLVM includes
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/InstrTypes.h"


using namespace llvm;
#define OPTIMIZABLE_SECTION_FLAG "OPTIMIZABLE_SECTION"

bool isOptimizable(Function* F) {
    if (!F->hasSection())
        return false;

    StringRef section = F->getSection();
    
    if (section == OPTIMIZABLE_SECTION_FLAG) {
        return true;
    }
    
    return false;
}

bool isArithmeticInstruction(llvm::Instruction &I) {
    return llvm::isa<llvm::BinaryOperator>(&I) &&
           (I.getOpcode() == llvm::Instruction::Add ||
            I.getOpcode() == llvm::Instruction::Sub ||
            I.getOpcode() == llvm::Instruction::Mul ||
            I.getOpcode() == llvm::Instruction::UDiv ||
            I.getOpcode() == llvm::Instruction::SDiv);
}

class SearchSpaceMeasurer {
private:
    std::unique_ptr<Module> module;
    LLVMContext context;
    std::map<std::string, double> mutation_search_spaces;
    
    // Helper function to count valid functions
    int countValidFunctions() {
        int count = 0;
        for (Function &F : *module) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                count++;
            }
        }
        return count;
    }
    
    // Helper function to count basic blocks in a function
    int countBasicBlocks(Function* func) {
        int count = 0;
        for (BasicBlock &BB : *func) {
            count++;
        }
        return count;
    }
    
    // Helper function to count instructions in a basic block
    int countInstructions(BasicBlock* BB) {
        int count = 0;
        for (Instruction &I : *BB) {
            count++;
        }
        return count;
    }
    
    // Helper function to count numeric instructions in a basic block
    int countNumericInstructions(BasicBlock* BB) {
        int count = 0;
        for (Instruction &I : *BB) {
            if (I.getType()->isIntegerTy() || I.getType()->isFloatingPointTy()) {
                count++;
            }
        }
        return count;
    }
    
    // Helper function to count non-terminator instructions in a basic block
    int countNonTerminatorInstructions(BasicBlock* BB) {
        int count = 0;
        for (Instruction &I : *BB) {
            if (!I.isTerminator()) {
                count++;
            }
        }
        return count;
    }
    
    // Helper function to count arithmetic instructions in a basic block
    int countArithmeticInstructions(BasicBlock* BB) {
        int count = 0;
        for (Instruction &I : *BB) {
            if (isArithmeticInstruction(I)) {
                count++;
            }
        }
        return count;
    }
    
    // Helper function to count loops in a function (simplified - count branches that could form loops)
    int countLoops(Function* func) {
        int count = 0;
        for (BasicBlock &BB : *func) {
            for (Instruction &I : BB) {
                // Count conditional branches as potential loop structures
                if (isa<BranchInst>(I)) {
                    BranchInst* branch = cast<BranchInst>(&I);
                    if (branch->isConditional()) {
                        count++;
                    }
                }
            }
        }
        return count;
    }

public:
    SearchSpaceMeasurer(const std::string& ll_file) {
        SMDiagnostic Err;
        module = parseAssemblyFile(ll_file, Err, context);
        if (!module) {
            std::cerr << "Error loading LLVM IR file: " << ll_file << std::endl;
            exit(1);
        }
    }
    
    // Measure search space for AddRandomArithmetic mutation
    double measureAddRandomArithmetic() {
        double total_search_space = 0.0;
        int valid_functions = countValidFunctions();
        
        if (valid_functions == 0) {
            return 0.0;
        }
        
        for (Function &F : *module) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                int basic_blocks = countBasicBlocks(&F);
                if (basic_blocks == 0) continue;
                
                for (BasicBlock &BB : F) {
                    int numeric_instructions = countNumericInstructions(&BB);
                    if (numeric_instructions < 2) continue;
                    
                    // Decision 1: Select function (valid_functions choices)
                    // Decision 2: Select basic block (basic_blocks choices)
                    // Decision 3: Select first instruction (numeric_instructions - 1 choices)
                    // Decision 4: Select second instruction (remaining choices)
                    // Decision 5: Select arithmetic operation (5 choices: Add, Sub, Mul, UDiv, SDiv)
                    // Decision 6: Select which instruction to replace (2 choices)
                    
                    double search_space = valid_functions * basic_blocks;
                    
                    // For each valid basic block, calculate combinations of numeric instructions
                    for (int i = 0; i < numeric_instructions - 1; i++) {
                        for (int j = i + 1; j < numeric_instructions; j++) {
                            search_space *= 5 * 2; // 5 arithmetic ops * 2 replacement choices
                        }
                    }
                    
                    total_search_space += search_space;
                }
            }
        }
        
        return total_search_space;
    }
    
    // Measure search space for DeleteRandomInstruction mutation
    double measureDeleteRandomInstruction() {
        double total_search_space = 0.0;
        int valid_functions = countValidFunctions();
        
        if (valid_functions == 0) {
            return 0.0;
        }
        
        for (Function &F : *module) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                int basic_blocks = countBasicBlocks(&F);
                if (basic_blocks == 0) continue;
                
                for (BasicBlock &BB : F) {
                    int non_terminator_instructions = countNonTerminatorInstructions(&BB);
                    if (non_terminator_instructions == 0) continue;
                    
                    // Decision 1: Select function (valid_functions choices)
                    // Decision 2: Select basic block (basic_blocks choices)
                    // Decision 3: Select instruction to delete (non_terminator_instructions choices)
                    
                    double search_space = valid_functions * basic_blocks * non_terminator_instructions;
                    total_search_space += search_space;
                }
            }
        }
        
        return total_search_space;
    }
    
    // Measure search space for MoveBlockwise mutation
    double measureMoveBlockwise() {
        double total_search_space = 0.0;
        int valid_functions = countValidFunctions();
        
        if (valid_functions == 0) {
            return 0.0;
        }
        
        for (Function &F : *module) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                int basic_blocks = countBasicBlocks(&F);
                if (basic_blocks < 2) continue; // Need at least 2 blocks to move
                
                for (BasicBlock &BB : F) {
                    int instructions_in_block = countInstructions(&BB);
                    if (instructions_in_block == 0) continue;
                    
                    // Decision 1: Select function (valid_functions choices)
                    // Decision 2: Select source basic block (basic_blocks choices)
                    // Decision 3: Select target basic block (basic_blocks - 1 choices, excluding source)
                    // Decision 4: Select instruction to move (instructions_in_block choices)
                    // Decision 5: Select position in target block (instructions_in_target + 1 choices)
                    
                    double search_space = valid_functions * basic_blocks * (basic_blocks - 1) * instructions_in_block;
                    
                    // For each target block, add position choices
                    for (BasicBlock &TargetBB : F) {
                        if (&TargetBB != &BB) {
                            int target_instructions = countInstructions(&TargetBB);
                            search_space *= (target_instructions + 1); // +1 for inserting at end
                        }
                    }
                    
                    total_search_space += search_space;
                }
            }
        }
        
        return total_search_space;
    }
    
    // Measure search space for AddNewCond mutation
    double measureAddNewCond() {
        double total_search_space = 0.0;
        int valid_functions = countValidFunctions();
        
        if (valid_functions == 0) {
            return 0.0;
        }
        
        for (Function &F : *module) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                int basic_blocks = countBasicBlocks(&F);
                if (basic_blocks < 2) continue; // Need at least 2 blocks for branching
                
                for (BasicBlock &BB : F) {
                    int arithmetic_instructions = countArithmeticInstructions(&BB);
                    if (arithmetic_instructions == 0) continue;
                    
                    // Decision 1: Select function (valid_functions choices)
                    // Decision 2: Select basic block (basic_blocks choices)
                    // Decision 3: Select arithmetic instruction (arithmetic_instructions choices)
                    // Decision 4: Select condition type (2 choices: true/false)
                    // Decision 5: Select target block for true branch (basic_blocks - 1 choices)
                    // Decision 6: Select target block for false branch (basic_blocks - 2 choices)
                    
                    double search_space = valid_functions * basic_blocks * arithmetic_instructions * 2;
                    search_space *= (basic_blocks - 1) * (basic_blocks - 2);
                    
                    total_search_space += search_space;
                }
            }
        }
        
        return total_search_space;
    }
    
    // Measure search space for SplitLoop mutation
    double measureSplitLoop() {
        double total_search_space = 0.0;
        int valid_functions = countValidFunctions();
        
        if (valid_functions == 0) {
            return 0.0;
        }
        
        for (Function &F : *module) {
            if (!F.isDeclaration() && isOptimizable(&F)) {
                int loops = countLoops(&F);
                if (loops == 0) continue;
                
                int basic_blocks = countBasicBlocks(&F);
                
                // Decision 1: Select function (valid_functions choices)
                // Decision 2: Select loop (loops choices)
                // Decision 3: Select split point (basic_blocks choices)
                // Decision 4: Select split type (2 choices: unroll/fission)
                
                double search_space = valid_functions * loops * basic_blocks * 2;
                total_search_space += search_space;
            }
        }
        
        return total_search_space;
    }
    
    // Measure all mutations and return average
    double measureAllMutations() {
        std::vector<double> search_spaces;
        
        search_spaces.push_back(measureAddRandomArithmetic());
        search_spaces.push_back(measureDeleteRandomInstruction());
        search_spaces.push_back(measureMoveBlockwise());
        search_spaces.push_back(measureAddNewCond());
        search_spaces.push_back(measureSplitLoop());
        
        // Calculate average
        double sum = 0.0;
        int count = 0;
        for (double space : search_spaces) {
            if (space > 0) {
                sum += space;
                count++;
            }
        }
        
        return count > 0 ? sum / count : 0.0;
    }
    
    // Print detailed results
    void printDetailedResults() {
        std::cout << "=== Search Space Measurement Results ===" << std::endl;
        std::cout << "File: " << module->getSourceFileName() << std::endl;
        std::cout << "Valid functions: " << countValidFunctions() << std::endl;
        std::cout << std::endl;
        
        std::cout << "Individual mutation search spaces:" << std::endl;
        std::cout << "  AddRandomArithmetic: " << measureAddRandomArithmetic() << std::endl;
        std::cout << "  DeleteRandomInstruction: " << measureDeleteRandomInstruction() << std::endl;
        std::cout << "  MoveBlockwise: " << measureMoveBlockwise() << std::endl;
        std::cout << "  AddNewCond: " << measureAddNewCond() << std::endl;
        std::cout << "  SplitLoop: " << measureSplitLoop() << std::endl;
        std::cout << std::endl;
        
        std::cout << "Average search space size: " << measureAllMutations() << std::endl;
        std::cout << "Log10 of average: " << log10(measureAllMutations()) << std::endl;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ll_file>" << std::endl;
        std::cerr << "Example: " << argv[0] << " input.ll" << std::endl;
        return 1;
    }
    
    std::string ll_file = argv[1];
    
    try {
        SearchSpaceMeasurer measurer(ll_file);
        measurer.printDetailedResults();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
