#ifndef MUTATION_FACTORY_CPP
#define MUTATION_FACTORY_CPP

#include "mutation.h"
#include "add_random_arithmetic.cpp"
#include "replace_arithmetic.cpp"
#include "move_blockwise.cpp"
#include "add_new_cond.cpp"
#include <random>
#include "constants.h"



void applyRandomMutation(Run& run_instance) {
    AddRandomArithmetic addRandomArithmetic;
    ReplaceArithmetic replaceArithmetic;
    MoveBlockwise moveBlockwise;
    AddNewCond addNewCond;
    std::uniform_int_distribution<> mutationTypeDistribution(0, 3); 
    // Randomly select which mutation to run
    int mutationTypeVal = mutationTypeDistribution(gen);
    
    std::vector<int> decisions;
    MutationType mutationType = static_cast<MutationType>(mutationTypeVal);
    switch (mutationType) {
        case ADD_RANDOM_ARITHMETIC:
            decisions = addRandomArithmetic.run(
                MODIFIED_CODE,
                MODIFIED_CODE
            );
            break;
        case REPLACE_ARITHMETIC:
            decisions = replaceArithmetic.run(
                MODIFIED_CODE,
                MODIFIED_CODE
            );
            break;
        case MOVE_BLOCKWISE:
            decisions = moveBlockwise.run(
                MODIFIED_CODE,
                MODIFIED_CODE
            );
            break;
        case ADD_NEW_COND:
            decisions = addNewCond.run(
                MODIFIED_CODE,
                MODIFIED_CODE
            );
            break;
    }
    run_instance.mutations.push_back(std::make_tuple(mutationType, decisions));
}

void reapplyMutation(Run& run_instance, MutationType mutationType, const std::vector<int>& decisions) {
    // Safety check: empty decisions vector
    if (decisions.empty()) {
        llvm::outs() << "Warning: Empty decisions vector for mutation type " << mutationType << "\n";
        return;
    }
    
    // Convert vector to array for the constructors
    int* decisionsArray = nullptr;
    try {
        decisionsArray = new int[decisions.size()];
        for (size_t i = 0; i < decisions.size(); i++) {
            decisionsArray[i] = decisions[i];
        }
        
        std::vector<int> result_decisions;
        bool success = false;
        
        switch (mutationType) {
            case ADD_RANDOM_ARITHMETIC: {
                AddRandomArithmetic addRandomArithmeticCustom(decisionsArray);
                result_decisions = addRandomArithmeticCustom.run(MODIFIED_CODE, MODIFIED_CODE);
                success = !result_decisions.empty();
                break;
            }
            case REPLACE_ARITHMETIC: {
                ReplaceArithmetic replaceArithmeticCustom(decisionsArray);
                result_decisions = replaceArithmeticCustom.run(MODIFIED_CODE, MODIFIED_CODE);
                success = !result_decisions.empty();
                break;
            }
            case MOVE_BLOCKWISE: {
                MoveBlockwise moveBlockwiseCustom(decisionsArray);
                result_decisions = moveBlockwiseCustom.run(MODIFIED_CODE, MODIFIED_CODE);
                success = !result_decisions.empty();
                break;
            }
            case ADD_NEW_COND: {
                AddNewCond addNewCondCustom(decisionsArray);
                result_decisions = addNewCondCustom.run(MODIFIED_CODE, MODIFIED_CODE);
                success = !result_decisions.empty();
                break;
            }
            default:
                llvm::outs() << "Warning: Unknown mutation type " << mutationType << "\n";
                break;
        }
        
        if (success) {
            run_instance.mutations.push_back(std::make_tuple(mutationType, result_decisions));
        } else {
            llvm::outs() << "Warning: Mutation application failed for type " << mutationType << "\n";
        }
    } catch (const std::exception& e) {
        llvm::outs() << "Exception in reapplyMutation: " << e.what() << "\n";
    } catch (...) {
        llvm::outs() << "Unknown exception in reapplyMutation\n";
    }
    
    // Clean up
    if (decisionsArray) {
        delete[] decisionsArray;
    }
}
#endif 