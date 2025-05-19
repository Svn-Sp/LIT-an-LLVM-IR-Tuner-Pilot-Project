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
#include <algorithm> // For sorting functions
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

#define exploration_share 0.5
#define elitism_share 1

void genetic_tuning(){
    std::vector<std::tuple<double, double, std::string>> results;
    std::vector<Run> runs;
    // Function to copy content from original.ll to modified.ll
    std::string result;
    int random_runs = RUN_COUNT * exploration_share;
    for (int t = 0; t < random_runs; t++){
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

    std::vector<Run> valid_runs;
    for (const Run& run : runs) {
        if (std::isfinite(run.result)) {
            valid_runs.push_back(run);
        }
    }
    if(valid_runs.size() < 1){
        llvm::outs() << "less than 1 valid runs found\n";
        valid_runs = runs;
    }
    
    std::sort(valid_runs.begin(), valid_runs.end(), [](const Run& a, const Run& b) {
        return a.avgDuration < b.avgDuration;
    });
    llvm::outs() <<"Starting building elite\n";
    int elite_count = std::max(1, static_cast<int>(valid_runs.size() * elitism_share));
    std::vector<Run> elite_runs(valid_runs.begin(), valid_runs.begin() + elite_count);
    
    for (int t = random_runs; t < RUN_COUNT; t++){
        llvm::outs() << "----------------------------------------\n";
        llvm::outs() << "Running genetic test " << t << "\n";
        
        llvm::outs() << "elite_runs.size(): " << elite_runs.size() << "\n";
        std::uniform_int_distribution<> eliteDistribution(0, elite_runs.size() - 1);
        int elite_idx = eliteDistribution(gen);
        Run& elite_run = elite_runs[elite_idx];
        std::uniform_int_distribution<> stepDistribution(
            elite_run.mutations.size() / 2, 
            elite_run.mutations.size()
        );
        int steps_to_keep = stepDistribution(gen);
        
        copyOriginalToModified(ORIGINAL_CODE, MODIFIED_CODE);
        
        Run run_instance;
        
        // Apply the first n steps from the elite run
        for (int i = 0; i < steps_to_keep; i++) {
            llvm::outs() << "Mutation step: " << i << "\n";
            llvm::outs() << elite_run.asString() << "\n";
            auto mutation = elite_run.mutations[i];
            MutationType type = std::get<0>(mutation);
            std::vector<int> decisions = std::get<1>(mutation);
            if(decisions.size() == 0){
                llvm::outs() << "No decisions found for mutation\n";
                continue;
            }
            llvm::outs() << "Reapplying mutation: " << type << "\n";
            reapplyMutation(run_instance, type, decisions);
            run_instance.mutations.push_back(mutation);
        }
        
        std::uniform_int_distribution<> newMutationsDistribution(1, 25);
        int new_mutations = newMutationsDistribution(gen);
        
        for (int i = 0; i < new_mutations; i++) {
            applyRandomMutation(run_instance);
        }
        
        //Evaluate the performance
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
        
        // If this run performed better than any of the elite runs, replace the worst elite run
        if (!result.empty() && avg < elite_runs.back().avgDuration) {
            // Remove the worst elite run
            elite_runs.pop_back();
            // Add this new better run
            elite_runs.push_back(run_instance);
            // Re-sort the elite runs
            std::sort(elite_runs.begin(), elite_runs.end(), [](const Run& a, const Run& b) {
                return a.avgDuration < b.avgDuration;
            });
            
            llvm::outs() << "Found a better run! Updated elite pool.\n";
        }
    }
    writeResultsToCSV("genetic_results.csv", results);
}