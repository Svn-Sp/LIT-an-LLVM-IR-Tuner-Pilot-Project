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
#include "add_random_arithmetic.cpp"
#include "replace_arithmetic.cpp"
#include "move_blockwise.cpp"
#include <csetjmp>  // Added for setjmp/longjmp
#include <sstream>  // Add this for stringstream
#include "file_management.cpp"
#include "call_executable.cpp"
#define BUFSIZE 128
#define ORIGINAL_CODE "original.ll"
#define MODIFIED_CODE "modified.ll"
#define RESULTS_FILE "results.csv"
#define RUN_COUNT 1000

enum MutationType {
    ADD_RANDOM_ARITHMETIC,
    REPLACE_ARITHMETIC,
    MOVE_BLOCKWISE
};

class Run {
    public:
    std::vector<std::tuple<MutationType, std::vector<int>>> mutations;
    double avgDuration;
    double stddevDuration;
    std::string asString(){
        std::stringstream ss;
        ss << "Run: " << avgDuration << " " << stddevDuration << "\n";
        for (std::tuple<MutationType, std::vector<int>> mutation : mutations) {
            ss << std::get<0>(mutation) << " ";
            for(int i : std::get<1>(mutation)){
                ss << i << " ";
            }
            ss << "\n";
        }
        return ss.str();
    }
    
};

std::jmp_buf jump_buffer;
std::random_device rd;
std::mt19937 gen(rd());
AddRandomArithmetic addRandomArithmetic;
ReplaceArithmetic replaceArithmetic;
MoveBlockwise moveBlockwise;

void signalHandler(int signal_num) {
    
    switch(signal_num) {
        case SIGSEGV:
            break;
        case SIGABRT:
            break;
        case SIGFPE:
            break;
        case SIGILL:
            break;
        default:
    }
    
    // Jump back to the saved point in main
    std::longjmp(jump_buffer, 1);
}

// Set up signal handlers using sigaction
void setupSignalHandlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    // SA_NODEFER ensures the signal handler remains installed during handler execution
    // SA_RESTART automatically restarts interrupted system calls
    sa.sa_flags = SA_NODEFER | SA_RESTART;
    
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
}

void applyRandomMutation(Run& run_instance){
    std::uniform_int_distribution<> mutationTypeDistribution(0, 2);
    // Randomly select which mutation to run
    int mutationTypeVal = mutationTypeDistribution(gen);
    
    std::vector<int> decisions;
    MutationType mutationType = static_cast<MutationType>(mutationTypeVal);
    switch (mutationType) {
        case ADD_RANDOM_ARITHMETIC:
            decisions = addRandomArithmetic.run(
                MODIFIED_CODE,
                MODIFIED_CODE
            );
            break;
        case REPLACE_ARITHMETIC:
            decisions = replaceArithmetic.run(
                MODIFIED_CODE,
                MODIFIED_CODE
            );
            break;
        case MOVE_BLOCKWISE:
            decisions = moveBlockwise.run(
                MODIFIED_CODE,
                MODIFIED_CODE
            );
            break;
    }
    run_instance.mutations.push_back(std::make_tuple(mutationType, decisions));
}


int main(int argc, char** argv) {
    // Set up signal handlers using sigaction
    setupSignalHandlers();
    // Create mutation instances with appropriate decision counts
    std::vector<std::tuple<double, double, std::string>> results;
    std::vector<Run> runs;
    // Function to copy content from original.ll to modified.ll
    std::string result;
    for (int t = 0; t < 500; t++){
        llvm::outs() << "----------------------------------------\n";
        llvm::outs() << "Running test " << t << "\n";
        copyOriginalToModified(ORIGINAL_CODE, MODIFIED_CODE);
        std::uniform_int_distribution<> mutationCountDistribution(3, 100);
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
        runs.push_back(run_instance);
        llvm::outs() << run_instance.asString() << "\n";
    }  
    writeResultsToCSV(RESULTS_FILE, results);
}