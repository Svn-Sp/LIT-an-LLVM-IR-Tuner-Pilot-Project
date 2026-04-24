#ifndef MUTATION_FACTORY_CPP
#define MUTATION_FACTORY_CPP

#include "mutations/mutation.h"
#include "core/run.h"
#include "utils/randomness_utils.cpp"
#include "mutations/add_random_arithmetic.cpp"
#include "mutations/move_blockwise.cpp"
#include "mutations/add_new_cond.cpp"
#include "mutations/delete_random_instruction.cpp"
#include "mutations/replace_with_dominating_value.cpp"
#include <random>
#include "constants.h"

extern std::mt19937 gen;

std::tuple<MutationType, std::vector<int>> applyRandomMutation(Run& run_instance, std::string modified_file) {
    static const std::vector<MutationType> allowedMutations = {
        ADD_RANDOM_ARITHMETIC,
        MOVE_BLOCKWISE,
        ADD_NEW_COND,
        DELETE_RANDOM_INSTRUCTION,
        REPLACE_WITH_DOMINATING_VALUE,
    };
    std::uniform_int_distribution<int> dist(0, allowedMutations.size() - 1);
    MutationType mutationType = allowedMutations[dist(gen)];
    llvm::outs() << "Apply mutation: " << mutationType << "\n";

    std::vector<int> decisions;
    switch (mutationType) {
        case ADD_RANDOM_ARITHMETIC: {
            AddRandomArithmetic m;
            decisions = m.run(modified_file.c_str(), modified_file.c_str());
            break;
        }
        case MOVE_BLOCKWISE: {
            MoveBlockwise m;
            decisions = m.run(modified_file.c_str(), modified_file.c_str());
            break;
        }
        case ADD_NEW_COND: {
            AddNewCond m;
            decisions = m.run(modified_file.c_str(), modified_file.c_str());
            break;
        }
        case DELETE_RANDOM_INSTRUCTION: {
            DeleteRandomInstruction m;
            decisions = m.run(modified_file.c_str(), modified_file.c_str());
            break;
        }
        case REPLACE_WITH_DOMINATING_VALUE: {
            ReplaceWithDominatingValue m;
            decisions = m.run(modified_file.c_str(), modified_file.c_str());
            break;
        }
    }
    run_instance.mutations.push_back(std::make_tuple(mutationType, decisions));
    return std::make_tuple(mutationType, decisions);
}

void reapplyMutation(Run& run_instance, MutationType mutationType, const std::vector<int>& decisions, std::string modified_file) {
    if (decisions.empty()) {
        llvm::outs() << "Warning: Empty decisions vector for mutation type " << mutationType << "\n";
        throw std::runtime_error("Empty decisions vector for mutation type " + std::to_string(mutationType));
    }

    try {
        std::vector<int> result_decisions;
        bool success = false;
        switch (mutationType) {
            case ADD_RANDOM_ARITHMETIC: {
                AddRandomArithmetic m(decisions);
                result_decisions = m.run(modified_file.c_str(), modified_file.c_str());
                success = !result_decisions.empty();
                break;
            }
            case MOVE_BLOCKWISE: {
                MoveBlockwise m(decisions);
                result_decisions = m.run(modified_file.c_str(), modified_file.c_str());
                success = !result_decisions.empty();
                break;
            }
            case ADD_NEW_COND: {
                AddNewCond m(decisions);
                result_decisions = m.run(modified_file.c_str(), modified_file.c_str());
                success = !result_decisions.empty();
                break;
            }
            case DELETE_RANDOM_INSTRUCTION: {
                DeleteRandomInstruction m(decisions);
                result_decisions = m.run(modified_file.c_str(), modified_file.c_str());
                success = !result_decisions.empty();
                break;
            }
            case REPLACE_WITH_DOMINATING_VALUE: {
                ReplaceWithDominatingValue m(decisions);
                result_decisions = m.run(modified_file.c_str(), modified_file.c_str());
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
}
#endif 