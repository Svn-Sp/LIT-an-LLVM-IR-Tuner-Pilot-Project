#ifndef FILE_MANAGEMENT_CPP
#define FILE_MANAGEMENT_CPP

#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include "core/run.h"

std::string sanitizeFileName(std::string file_name) {
    std::string sanitized_file_name = file_name;
    std::replace(sanitized_file_name.begin(), sanitized_file_name.end(), '/', '_');
    std::replace(sanitized_file_name.begin(), sanitized_file_name.end(), '.', '_');
    return sanitized_file_name;
}

void copyOriginalToModified(std::string original_code, std::string modified_code) {
    std::ifstream src(original_code, std::ios::binary);
    std::ofstream dst(modified_code, std::ios::binary);
    
    if (!src) {
        return;
    }
    
    if (!dst) {
        return;
    }
    
    dst << src.rdbuf();
    
    src.close();
    dst.close();
}

int writeResultsToCSV(std::string filename, std::vector<Run> results) {
    std::ofstream csvFile(filename);
    if (csvFile.is_open()) {
        // Write header
        csvFile << "Run,Success,Average Duration (s),Standard Deviation (s),Result\n";
        
        // Write data
        for (size_t i = 0; i < results.size(); i++) {
            csvFile << i << "," 
                   << results[i].success << ","
                   << results[i].avgDuration << "," 
                   << results[i].stddevDuration << "," 
                   << results[i].result << "\n";
        }
        
        csvFile.close();
    } else {
        llvm::errs() << "Error: Could not open results.csv for writing\n";
        return 1;
    }
    return 0;
}

#endif // FILE_MANAGEMENT_CPP