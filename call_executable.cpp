#ifndef CALL_EXECUTABLE_CPP
#define CALL_EXECUTABLE_CPP

#include <sys/select.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <cstdio>

#define TIMEOUT_SECONDS 5

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

#endif // CALL_EXECUTABLE_CPP