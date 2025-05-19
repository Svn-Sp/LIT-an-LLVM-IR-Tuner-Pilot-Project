#ifndef MEASURE_TIME_CPP
#define MEASURE_TIME_CPP

#include <sys/select.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <cstdio>
#include <chrono>
#include <cmath>
#include "core.cpp"

#define TIMEOUT_SECONDS 5
#define REPETITIONS 2

int call_executable(const std::string& cmd, std::string* output, int timeout_seconds = TIMEOUT_SECONDS) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return -1;  // Error opening pipe
    }
    
    // Set up file descriptor for the pipe
    int fd = fileno(pipe);
    fd_set fds;
    struct timeval tv;
    
    char buffer[1024];
    output->clear();
    bool timeout_occurred = false;
    
    while (true) {
        // Set up the file descriptor set
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        
        // Set the timeout
        tv.tv_sec = timeout_seconds;
        tv.tv_usec = 0;
        
        // Wait for data or timeout
        int ret = select(fd + 1, &fds, NULL, NULL, &tv);
        
        if (ret == -1) {
            // Select error
            break;
        } else if (ret == 0) {
            // Timeout occurred
            
            // Mark timeout and break out of the loop
            timeout_occurred = true;
            break;
        } else {
            // Data available, read it
            size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, pipe);
            if (bytes_read <= 0) break;  // EOF or error
            
            buffer[bytes_read] = '\0';
            output->append(buffer);
        }
    }
    
    // Kill the process if timeout occurred
    if (timeout_occurred) {
        // Get process group ID to kill all children
        std::string kill_cmd = "pkill -f \"lli modified.ll\"";
        system(kill_cmd.c_str());
        
        // Allow a moment for the process to terminate
        usleep(100000);  // 100ms
    }
    
    // Always close the pipe at the end
    int status = pclose(pipe);
    
    return timeout_occurred ? 1 : WEXITSTATUS(status);
}

void measure_time(std::string& result, Run& run_instance, std::vector<std::tuple<double, double, std::string>>& results, const std::string& cmd = "lli modified.ll"){
    std::vector<double> durations;
    for (int run = 0; run < REPETITIONS; run++) {
        auto start_time = std::chrono::high_resolution_clock::now();
        call_executable(cmd, &result);
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
    run_instance.saveToDb();
}

#endif // MEASURE_TIME_CPP