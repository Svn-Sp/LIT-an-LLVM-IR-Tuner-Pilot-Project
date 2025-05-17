#pragma once
#include "decision_maker.cpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
using namespace llvm;


class Mutation{
    public:
        DecisionMaker dm;
        
        Mutation(int decisionsCount) : dm(decisionsCount) {
        }
        
        Mutation(int decisionsCount, int decisions[]) : dm(decisionsCount, decisions) {
        }
        
        std::vector<int> run(const char* input_file, const char* output_file){
            LLVMContext Context;

            // Parse input LLVM IR file
            SMDiagnostic Err;
            std::unique_ptr<Module> M = parseAssemblyFile(input_file, Err, Context);
        
            if (!M) {
                printf("Failed to parse %s\n", input_file);
                return {};
            }

            llvm::StringRef Identifier = M->getModuleIdentifier();
            std::unique_ptr<Module> updatedM = mutate(std::move(M));
            std::error_code EC;
            if(updatedM == nullptr){
                return {};
            }
            //Check if modified module will be parsable
            std::string irString;
            llvm::raw_string_ostream rso(irString);
            updatedM->print(rso, nullptr);
            rso.flush();

            llvm::LLVMContext TempContext;

            // Try to parse the IR from the string
            std::unique_ptr<llvm::Module> ParsedModule = llvm::parseIR(
                llvm::MemoryBufferRef(irString, "in-memory IR"), Err, TempContext);
            if(ParsedModule == nullptr){
                return {};
            }
            raw_fd_ostream Out(output_file, EC);
            updatedM->print(Out, nullptr);
            Out.close();
            
            // Return the decisions made during this mutation
            std::vector<int> decisions;
            for (int i = 0; i < dm.num_decisions; i++) {
                decisions.push_back(dm.decisions[i]);
            }
            return decisions;
        }
        virtual std::unique_ptr<Module> mutate(std::unique_ptr<Module> M) = 0;
};