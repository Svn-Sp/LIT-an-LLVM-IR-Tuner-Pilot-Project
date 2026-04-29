#ifndef MEASURE_TIME_CPP
#define MEASURE_TIME_CPP

#include <sys/select.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <chrono>
#include <cmath>
#include <limits>
#include <vector>
#include <typeinfo>
#include <filesystem>
#include <iostream>
#include <ctime>
#include "core/run.cpp"

#include "output/output.cpp"
#include "output/scalar.cpp"
#include "output/array.cpp"
#include "output/matrix2d.cpp"
#define TIMEOUT_SECONDS 6
#define REPETITIONS 20
#define COMPILED_BINARY "/tmp/lit_benchmark_binary"
#define COMPILED_OBJ    "/tmp/lit_benchmark_binary.o"

// Compile an LLVM IR file to a native binary at COMPILED_BINARY.
// Returns true on success.
bool compile_ll_to_binary(const std::string& ll_file) {
    std::string llc_cmd = "llc -O0 -filetype=obj " + ll_file + " -o " COMPILED_OBJ " 2>/dev/null";
    if (system(llc_cmd.c_str()) != 0) {
        llvm::outs() << "compile_ll_to_binary: llc failed for " << ll_file << "\n";
        return false;
    }
    std::string link_cmd = "clang++ -O0 -no-pie " COMPILED_OBJ " -o " COMPILED_BINARY " -lm 2>/dev/null";
    if (system(link_cmd.c_str()) != 0) {
        llvm::outs() << "compile_ll_to_binary: clang++ linking failed for " << ll_file << "\n";
        return false;
    }
    return true;
}

int call_executable(const std::string& program_file,  double& time_result, int timeout_seconds = TIMEOUT_SECONDS) {
    // Use fork and exec for better process control
    int time_fd[2];
    pipe(time_fd);
    pid_t pid = fork();
    
    if (pid == -1) {
        // Fork failed
        std::cerr << "Error: Failed to fork process" << std::endl;
        return -1;
    }
    
    if (pid == 0) {
        close(time_fd[0]);
        // Child process
        // Create a new process group to make it easier to kill all children
        setpgid(0, 0);
        
        // Redirect output to /dev/null to avoid hanging on output
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        
        // Execute the compiled binary
        std::string cmd = "taskset -c 5 " COMPILED_BINARY " > /dev/null 2>&1";
        auto start_time = std::chrono::high_resolution_clock::now();
        int ret = system(cmd.c_str());
        auto end_time = std::chrono::high_resolution_clock::now();
        if (ret != 0) {
            std::cerr << "Failed to run program: " << program_file << std::endl;
            exit(1);
        }
        std::chrono::duration<double> elapsed = end_time - start_time;
        double time = elapsed.count();
        write(time_fd[1], &time, sizeof(time));
        close(time_fd[1]);
        exit(0);
    } else {
        // Parent process
        int status;
        time_t start_time = time(nullptr);
        
        // Set the process group for the child
        setpgid(pid, pid);
        
        while (true) {
            // Check if process has finished
            pid_t result = waitpid(pid, &status, WNOHANG);
            
            if (result == pid) {
                // Process finished - ensure all children are killed
                killpg(pid, SIGTERM);
                sleep(1);
                
                // Force kill any remaining processes in the group
                if (killpg(pid, 0) == 0) {
                    killpg(pid, SIGKILL);
                    sleep(1);
                }
                
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    read(time_fd[0], &time_result, sizeof(time_result));
                    close(time_fd[0]);
                    return 1;  // Success
                } else {
                    return -1;  // Process failed
                }
            } else if (result == -1) {
                // Error in waitpid - kill the process group
                killpg(pid, SIGTERM);
                sleep(1);
                
                // Force kill if still running
                if (killpg(pid, 0) == 0) {
                    killpg(pid, SIGKILL);
                    sleep(1);
                }
                
                std::cerr << "Error: waitpid failed" << std::endl;
                return -1;
            }
            
            // Check timeout
            if (time(nullptr) - start_time >= timeout_seconds) {
                // Timeout - kill the process group
                killpg(pid, SIGTERM);
                sleep(1);
                
                // Force kill if still running
                if (killpg(pid, 0) == 0) {
                    killpg(pid, SIGKILL);
                    sleep(1);
                }
                
                std::cerr << "Error: Process timed out after " << timeout_seconds << " seconds" << std::endl;
                return -1;
            }
            
            // Sleep a bit before checking again
            usleep(10000);  // 10ms
        }
    }
}

// Helper function to kill any lingering lli processes
void cleanup_lli_processes() {
    // Kill any remaining lli processes that might be hanging
    system("pkill -f 'lli ' > /dev/null 2>&1");
    usleep(100000);  // 100ms to let processes terminate
}

static double compute_distance(const std::string& output_file, OutputBase& correct_result) {
    if (typeid(correct_result) == typeid(Scalar)) {
        Scalar result(output_file);
        return result.get_distance(correct_result);
    } else if (typeid(correct_result) == typeid(ArrayOutput)) {
        ArrayOutput result(output_file);
        return result.get_distance(correct_result);
    } else if (typeid(correct_result) == typeid(Matrix2DOutput)) {
        Matrix2DOutput result(output_file);
        return result.get_distance(correct_result);
    }
    throw std::runtime_error("Output type not supported");
}

int measure_time(std::string program_file, std::string output_file, OutputBase& correct_result, Run& run_instance){
    // Compile the IR to a native binary once before timing
    if (!compile_ll_to_binary(program_file)) {
        llvm::outs() << "measure_time: compilation failed for " << program_file << "\n";
        run_instance.success = false;
        run_instance.avgDuration = 0;
        run_instance.stddevDuration = 0;
        run_instance.result = std::numeric_limits<double>::infinity();
        return -1;
    }

    std::vector<double> durations;
    std::vector<double> distances;

    for (int run = 0; run < REPETITIONS; run++) {
        if (std::filesystem::exists(output_file)) {
            std::filesystem::remove(output_file);
        }

        double time_result;
        int success = call_executable(program_file, time_result);
        cleanup_lli_processes();

        if (success != 1 || !std::filesystem::exists(output_file)) {
            llvm::outs() << "Run " << run << ": executable failed — aborting\n";
            run_instance.success = false;
            run_instance.avgDuration = 0;
            run_instance.stddevDuration = 0;
            run_instance.result = std::numeric_limits<double>::infinity();
            return -1;
        }

        durations.push_back(time_result);

        try {
            double dist = compute_distance(output_file, correct_result);
            llvm::outs() << "Run " << run << ": distance = " << dist << "\n";
            distances.push_back(dist);
        } catch (const std::exception& e) {
            llvm::outs() << "Run " << run << ": failed to parse output: " << e.what() << " — aborting\n";
            run_instance.success = false;
            run_instance.avgDuration = 0;
            run_instance.stddevDuration = 0;
            run_instance.result = std::numeric_limits<double>::infinity();
            return -1;
        }
    }

    cleanup_lli_processes();

    double sum = 0.0;
    for (double d : durations) sum += d;
    double avg = sum / durations.size();
    double variance = 0.0;
    for (double d : durations) variance += (d - avg) * (d - avg);
    variance /= durations.size();

    double max_distance = 0.0;
    for (double d : distances) max_distance = std::max(max_distance, d);

    run_instance.success = true;
    run_instance.avgDuration = avg;
    run_instance.stddevDuration = std::sqrt(variance);
    run_instance.result = max_distance;

    llvm::outs() << "Final distance: " << run_instance.result
                 << " (all " << REPETITIONS << " runs succeeded)\n";
    return 1;
}

#endif // MEASURE_TIME_CPP