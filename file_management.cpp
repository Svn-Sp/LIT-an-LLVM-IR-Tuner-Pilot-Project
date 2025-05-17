#include <fstream>
#include <string>

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



int writeResultsToCSV(std::string filename, std::vector<std::tuple<double, double, std::string>> results) {
    std::ofstream csvFile(filename);
    if (csvFile.is_open()) {
        // Write header
        csvFile << "Run,Average Duration (s),Standard Deviation (s),Result\n";
        
        // Write data
        for (size_t i = 0; i < results.size(); i++) {
            csvFile << i << "," 
                   << std::get<0>(results[i]) << "," 
                   << std::get<1>(results[i]) << "," 
                   << std::get<2>(results[i]) << "\n";
        }
        
        csvFile.close();
    } else {
        llvm::errs() << "Error: Could not open results.csv for writing\n";
        return 1;
    }
    return 0;
}