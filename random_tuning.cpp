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
#include "call_executable.cpp"
#include "core.cpp"
#include "constants.h"
#include "randomness_utils.cpp"
#include "mutation_factory.cpp"

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
        // Call executable three times and measure durations
        std::vector<double> durations;
        for (int run = 0; run < 3; run++) {
            auto start_time = std::chrono::high_resolution_clock::now();
            call_executable("lli modified.ll", &result);
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;
            durations.push_back(elapsed.count());
        }
        double sum = 0.0;
        for (double d : durations) sum += d;
        double avg = sum / durations.size();
        
        double variance = 0.0;
        for (double d : durations) variance += (d - avg) * (d - avg);
        variance /= durations.size();
        double stddev = std::sqrt(variance);
        results.push_back(std::make_tuple(avg, stddev, result));
        run_instance.avgDuration = avg;
        run_instance.stddevDuration = stddev;
        try {
            run_instance.result = std::stod(result);
        } catch (const std::invalid_argument& e) {
        } catch (const std::out_of_range& e) {
        }
        runs.push_back(run_instance);
        llvm::outs() << run_instance.asString() << "\n";
    }  
    writeResultsToCSV(RESULTS_FILE, results);
}