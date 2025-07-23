#ifndef MEASURE_TIME_CPP
#define MEASURE_TIME_CPP

#include <sys/select.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <cstdio>
#include <chrono>
#include <cmath>
#include <vector>
#include <typeinfo>
#include <filesystem>
#include <iostream>
#include <ctime>
#include "core/run.cpp"
#include "output/output.cpp"
#include "output/scalar.cpp"
#include "output/array.cpp"

#define TIMEOUT_SECONDS 3
#define REPETITIONS 5

int call_executable(const std::string& cmd, int timeout_seconds = TIMEOUT_SECONDS) {
    // Use fork and exec for better process control
    pid_t pid = fork();
    
    if (pid == -1) {
        // Fork failed
        std::cerr << "Error: Failed to fork process" << std::endl;
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        // Redirect output to /dev/null to avoid hanging on output
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        
        // Parse the command (assuming format: "lli program_file")
        size_t space_pos = cmd.find(' ');
        if (space_pos == std::string::npos) {
            std::cerr << "Error: Invalid command format: " << cmd << std::endl;
            exit(1);
        }
        
        std::string program_file = cmd.substr(space_pos + 1);
        
        // Execute the command
        execlp("bash", "bash", "-c", ("taskset -c 0 /usr/bin/time -f \"%e\" lli \"" + program_file + "\" > /dev/null 2>&1").c_str(), nullptr);
        
        // If we get here, exec failed
        std::cerr << "Error: Failed to execute command: " << cmd << std::endl;
        exit(1);
    } else {
        // Parent process
        int status;
        time_t start_time = time(nullptr);
        
        while (true) {
            // Check if process has finished
            pid_t result = waitpid(pid, &status, WNOHANG);
            
            if (result == pid) {
                // Process finished
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    return 1;  // Success
                } else {
                    return -1;  // Process failed
                }
            } else if (result == -1) {
                // Error in waitpid
                std::cerr << "Error: waitpid failed" << std::endl;
                return -1;
            }
            
            // Check timeout
            if (time(nullptr) - start_time >= timeout_seconds) {
                // Timeout - kill the process
                kill(pid, SIGTERM);
                
                // Give it a moment to terminate gracefully
                sleep(1);
                
                // Force kill if still running
                if (waitpid(pid, &status, WNOHANG) == 0) {
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);  // Wait for it to die
                }
                
                std::cerr << "Error: Process timed out after " << timeout_seconds << " seconds" << std::endl;
                return -1;
            }
            
            // Sleep a bit before checking again
            usleep(10000);  // 10ms
        }
    }
}

int measure_time(std::string program_file, std::string output_file, OutputBase& correct_result, Run& run_instance){
    if (std::filesystem::exists(output_file)) {
        std::filesystem::remove(output_file);
    }
    std::vector<double> durations;
    for (int run = 0; run < REPETITIONS; run++) {
        auto start_time = std::chrono::high_resolution_clock::now();
        int success = call_executable("lli " + program_file);
        auto end_time = std::chrono::high_resolution_clock::now();
        if (success != 1 || !std::filesystem::exists(output_file)) {
            llvm::outs() << "Executable failed to run\n";
            run_instance.success = false;
            run_instance.avgDuration = 0;
            run_instance.stddevDuration = 0;
            run_instance.result = 0;
            return -1;
        }
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
    
    run_instance.success = true;
    run_instance.avgDuration = avg;
    run_instance.stddevDuration = stddev;
    if (typeid(correct_result) == typeid(Scalar)) {
        Scalar result(output_file);
        double distance = result.get_distance(correct_result);
        llvm::outs() << "Measured Distance: " << distance << "\n";
        run_instance.result = distance;
    } else if (typeid(correct_result) == typeid(ArrayOutput)) {
        ArrayOutput result(output_file);
        double distance = result.get_distance(correct_result);
        llvm::outs() << "Measured Distance: " << distance << "\n";
        run_instance.result = distance;
    } else {
        throw std::runtime_error("Output type not supported");
    }
    //run_instance.saveToDb();
    return 1;
}

#endif // MEASURE_TIME_CPP