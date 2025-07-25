#ifndef MUTATION_FACTORY_CPP
#define MUTATION_FACTORY_CPP

#include "mutations/mutation.h"
#include "core/run.h"
#include "utils/randomness_utils.cpp"
#include "mutations/add_random_arithmetic.cpp"
#include "mutations/move_blockwise.cpp"
#include "mutations/unsafe_mem_2_reg.cpp"
#include "mutations/add_new_cond.cpp"
#include <random>
#include "constants.h"

extern std::mt19937 gen;

std::tuple<MutationType, std::vector<int>> applyRandomMutation(Run& run_instance, std::string modified_file) {
    AddRandomArithmetic addRandomArithmetic;
    MoveBlockwise moveBlockwise;
    AddNewCond addNewCond;
    UnsafeMem2Reg unsafeMem2Reg;
    std::uniform_int_distribution<> mutationTypeDistribution(0, 3);
    // Randomly select which mutation to run
    int mutationTypeVal = mutationTypeDistribution(gen);
    
    std::vector<int> decisions;
    MutationType mutationType = static_cast<MutationType>(mutationTypeVal);
    llvm::outs() << "Apply mutation: " << mutationType << "\n";
    switch (mutationType) {
        case ADD_RANDOM_ARITHMETIC:
            decisions = addRandomArithmetic.run(
                modified_file.c_str(),
                modified_file.c_str()
            );
            break;
        case MOVE_BLOCKWISE:
            decisions = moveBlockwise.run(
                modified_file.c_str(),
                modified_file.c_str()
            );
            break;
        case ADD_NEW_COND:
            decisions = addNewCond.run(
                modified_file.c_str(),
                modified_file.c_str()
            );
            break;
        case UNSAFE_MEM_2_REG:
            decisions = unsafeMem2Reg.run(
                modified_file.c_str(),
                modified_file.c_str()
            );
            break;
    }
    run_instance.mutations.push_back(std::make_tuple(mutationType, decisions));
    return std::make_tuple(mutationType, decisions);
}

void reapplyMutation(Run& run_instance, MutationType mutationType, const std::vector<int>& decisions, std::string modified_file) {
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
                result_decisions = addRandomArithmeticCustom.run(modified_file.c_str(), modified_file.c_str());
                success = !result_decisions.empty();
                break;
            }
            case MOVE_BLOCKWISE: {
                MoveBlockwise moveBlockwiseCustom(decisionsArray);
                result_decisions = moveBlockwiseCustom.run(modified_file.c_str(), modified_file.c_str());
                success = !result_decisions.empty();
                break;
            }
            case ADD_NEW_COND: {
                AddNewCond addNewCondCustom(decisionsArray);
                result_decisions = addNewCondCustom.run(modified_file.c_str(), modified_file.c_str());
                success = !result_decisions.empty();
                break;
            }
            case UNSAFE_MEM_2_REG: {
                UnsafeMem2Reg unsafeMem2RegCustom(decisionsArray);
                result_decisions = unsafeMem2RegCustom.run(modified_file.c_str(), modified_file.c_str());
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
            throw std::runtime_error("Mutation reapplication failed");
        }
    } catch (const std::exception& e) {
        llvm::outs() << "Exception in reapplyMutation: " << e.what() << "\n";
        throw std::runtime_error("Mutation reapplication failed");
    } catch (...) {
        llvm::outs() << "Unknown exception in reapplyMutation\n";
        throw std::runtime_error("Mutation reapplication failed");
    }
    
    // Clean up
    if (decisionsArray) {
        delete[] decisionsArray;
    }
}
#endif 