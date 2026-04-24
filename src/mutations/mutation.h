#ifndef MUTATION_H
#define MUTATION_H

#include "core/decision_maker.cpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include <vector>
#include <string>

using namespace llvm;

class Mutation {
public:
    DecisionMaker dm;

    Mutation();                                    // new-decisions mode
    Mutation(std::vector<int> decisions);          // replay mode

    std::vector<int> run(const char* input_file, const char* output_file);
    virtual std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) = 0;
};

enum MutationType {
    ADD_RANDOM_ARITHMETIC,
    MOVE_BLOCKWISE,
    ADD_NEW_COND,
    DELETE_RANDOM_INSTRUCTION,
    REPLACE_WITH_DOMINATING_VALUE,
};

// Add string conversion functionality
inline std::string mutation_type_to_string(MutationType type) {
    switch (type) {
        case ADD_RANDOM_ARITHMETIC: return "ADD_RANDOM_ARITHMETIC";
        case MOVE_BLOCKWISE: return "MOVE_BLOCKWISE";
        case ADD_NEW_COND: return "ADD_NEW_COND";
        case DELETE_RANDOM_INSTRUCTION: return "DELETE_RANDOM_INSTRUCTION";
        case REPLACE_WITH_DOMINATING_VALUE: return "REPLACE_WITH_DOMINATING_VALUE";
        default: return "UNKNOWN";
    }
}

inline MutationType stringToMutationType(const std::string& typeStr) {
    if (typeStr == "ADD_RANDOM_ARITHMETIC") return ADD_RANDOM_ARITHMETIC;
    if (typeStr == "MOVE_BLOCKWISE") return MOVE_BLOCKWISE;
    if (typeStr == "ADD_NEW_COND") return ADD_NEW_COND;
    if (typeStr == "DELETE_RANDOM_INSTRUCTION") return DELETE_RANDOM_INSTRUCTION;
    if (typeStr == "REPLACE_WITH_DOMINATING_VALUE") return REPLACE_WITH_DOMINATING_VALUE;
    std::cerr << "Warning: Unknown mutation type: " << typeStr << std::endl;
    throw std::runtime_error("Unknown mutation type: " + typeStr);
}

#endif // MUTATION_H 