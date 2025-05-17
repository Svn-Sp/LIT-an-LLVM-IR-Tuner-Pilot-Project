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