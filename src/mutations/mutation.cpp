#ifndef MUTATION_CPP
#define MUTATION_CPP

#include "mutations/mutation.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

Mutation::Mutation() : dm() {}

Mutation::Mutation(std::vector<int> decisions) : dm(std::move(decisions)) {}

std::vector<int> Mutation::run(const char* input_file, const char* output_file) {
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
        llvm::errs() << "Failed to mutate" << "\n";
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
        llvm::errs() << "Failed to parse modified module" << "\n";
        llvm::errs() << Err.getMessage() << "\n";
        return {};
    }
    raw_fd_ostream Out(output_file, EC);
    updatedM->print(Out, nullptr);
    Out.close();
    return dm.decisions;
}

#endif // MUTATION_CPP