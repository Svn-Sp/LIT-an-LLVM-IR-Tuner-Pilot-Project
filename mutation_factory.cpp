#ifndef MUTATION_FACTORY_CPP
#define MUTATION_FACTORY_CPP

#include "mutation.h"
#include "add_random_arithmetic.cpp"
#include "replace_arithmetic.cpp"
#include "move_blockwise.cpp"
#include <random>
#include "constants.h"

AddRandomArithmetic addRandomArithmetic;
ReplaceArithmetic replaceArithmetic;
MoveBlockwise moveBlockwise;


void applyRandomMutation(Run& run_instance) {
    std::uniform_int_distribution<> mutationTypeDistribution(0, 2);
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
    }
    run_instance.mutations.push_back(std::make_tuple(mutationType, decisions));
}

void reapplyMutation(Run& run_instance, MutationType mutationType, const std::vector<int>& decisions) {
    // Convert vector to array for the constructors
    int* decisionsArray = new int[decisions.size()];
    for (size_t i = 0; i < decisions.size(); i++) {
        decisionsArray[i] = decisions[i];
    }
    
    switch (mutationType) {
        case ADD_RANDOM_ARITHMETIC: {
            AddRandomArithmetic addRandomArithmeticCustom(decisionsArray);
            addRandomArithmeticCustom.run(MODIFIED_CODE, MODIFIED_CODE);
            break;
        }
        case REPLACE_ARITHMETIC: {
            ReplaceArithmetic replaceArithmeticCustom(decisionsArray);
            replaceArithmeticCustom.run(MODIFIED_CODE, MODIFIED_CODE);
            break;
        }
        case MOVE_BLOCKWISE: {
            MoveBlockwise moveBlockwiseCustom(decisionsArray);
            moveBlockwiseCustom.run(MODIFIED_CODE, MODIFIED_CODE);
            break;
        }
    }
    
    delete[] decisionsArray;
}
#endif 