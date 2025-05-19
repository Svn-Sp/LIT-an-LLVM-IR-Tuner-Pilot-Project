#include <signal.h> 
#include "llvm/Support/Error.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"  // Add this for outs()
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>  // Add this for timing measurements
#include <fstream>  // Add this for file operations
#include <setjmp.h>
#include <string>
#include <random>
#include "move_blockwise.cpp"
#include "mutation.h"
#include "replace_arithmetic.cpp"
#include "add_random_arithmetic.cpp"
#include <csetjmp>  // Added for setjmp/longjmp
#include <sstream>  // Add this for stringstream
#include "file_management.cpp"
#include "core.cpp"
#include "constants.h"
#include "randomness_utils.cpp"
#include "mutation_factory.cpp"
#include "measure_time.cpp"
void random_tuning(){
    std::vector<std::tuple<double, double, std::string>> results;
    std::vector<Run> runs;
    // Function to copy content from original.ll to modified.ll
    std::string result;
    for (int t = 0; t < RUN_COUNT; t++){
        llvm::outs() << "----------------------------------------\n";
        llvm::outs() << "Running test " << t << "\n";
        copyOriginalToModified(ORIGINAL_CODE, MODIFIED_CODE);
        std::uniform_int_distribution<> mutationCountDistribution(MIN_MUTATIONS, MAX_MUTATIONS);
        int mutationCount = mutationCountDistribution(gen);
        Run run_instance;
        for (int i = 0; i < mutationCount; i++) {
            applyRandomMutation(run_instance);
        }
        measure_time(result, run_instance, results);
        runs.push_back(run_instance);
        llvm::outs() << run_instance.asString() << "\n";
    }  
    writeResultsToCSV(RESULTS_FILE, results);
}