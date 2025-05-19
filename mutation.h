#ifndef MUTATION_H
#define MUTATION_H

#include "decision_maker.cpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include <vector>

using namespace llvm;

class Mutation {
public:
    DecisionMaker dm;
    
    Mutation(int decisionsCount);
    Mutation(int decisionsCount, int decisions[]);
    
    std::vector<int> run(const char* input_file, const char* output_file);
    virtual std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) = 0;
};

enum MutationType {
    ADD_RANDOM_ARITHMETIC,
    REPLACE_ARITHMETIC,
    MOVE_BLOCKWISE,
    ADD_NEW_COND
};

#endif // MUTATION_H 