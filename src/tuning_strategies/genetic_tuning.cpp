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
#include "mutations/move_blockwise.cpp"
#include "mutations/mutation.h"
#include "mutations/replace_arithmetic.cpp"
#include "mutations/add_random_arithmetic.cpp"
#include <csetjmp>  // Added for setjmp/longjmp
#include <sstream>  // Add this for stringstream
#include "utils/file_management.cpp"
#include "core/run.cpp"
#include "constants.h"
#include "utils/randomness_utils.cpp"
#include "mutations/mutation_factory.cpp"
#include "utils/measure_time.cpp"
#define exploration_share 1
#define elitism_share 1

void genetic_tuning(){
    std::vector<std::tuple<double, double, std::string>> results;
    std::vector<Run> runs;
    std::string result;
    Run original_run;
    measure_time(result, original_run, results, "lli original.ll");
    int random_runs = EVALUATIONS_BUDGET * exploration_share;
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
        measure_time(result, run_instance, results);
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
    
    for (int t = random_runs; t < EVALUATIONS_BUDGET; t++){
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
        
        runs.push_back(run_instance);
        llvm::outs() << run_instance.asString() << "\n";
        
        // If this run performed better than any of the elite runs, replace the worst elite run
        if (!result.empty() && run_instance.avgDuration < elite_runs.back().avgDuration) {
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