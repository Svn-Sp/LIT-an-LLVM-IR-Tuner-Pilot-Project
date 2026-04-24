#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <filesystem>
#include "mutations/mutation_factory.cpp"
#include "core/run.h"
#include "constants.h"

// Function to parse CSV line with proper quote handling
std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            tokens.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    
    // Add the last token
    tokens.push_back(current);
    
    return tokens;
}

// Function to parse decisions string "[0, 5, 38, 27]" -> vector<int>
std::vector<int> parseDecisions(const std::string& decisionsStr) {
    std::vector<int> decisions;
    std::string cleanStr = decisionsStr;
    
    // Remove brackets
    if (cleanStr.length() >= 2 && cleanStr[0] == '[' && cleanStr[cleanStr.length()-1] == ']') {
        cleanStr = cleanStr.substr(1, cleanStr.length()-2);
    }
    
    std::stringstream ss(cleanStr);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        // Remove whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        
        if (!token.empty()) {
            try {
                decisions.push_back(std::stoi(token));
            } catch (const std::exception& e) {
                std::cerr << "Warning: Could not parse decision value: " << token << std::endl;
            }
        }
    }
    
    return decisions;
}


// Function to apply a single mutation
bool applyMutation(MutationType mutationType, const std::vector<int>& decisions, 
                   const std::string& inputFile, const std::string& outputFile) {
    // Safety check: empty decisions vector
    if (decisions.empty()) {
        std::cerr << "Warning: Empty decisions vector for mutation type " << mutationType << std::endl;
        return false;
    }
    
    try {
        std::vector<int> result_decisions;
        bool success = false;

        switch (mutationType) {
            case ADD_RANDOM_ARITHMETIC: {
                AddRandomArithmetic m(decisions);
                result_decisions = m.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            case MOVE_BLOCKWISE: {
                MoveBlockwise m(decisions);
                result_decisions = m.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            case ADD_NEW_COND: {
                AddNewCond m(decisions);
                result_decisions = m.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            case DELETE_RANDOM_INSTRUCTION: {
                DeleteRandomInstruction m(decisions);
                result_decisions = m.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            case REPLACE_WITH_DOMINATING_VALUE: {
                ReplaceWithDominatingValue m(decisions);
                result_decisions = m.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            default:
                std::cerr << "Warning: Unknown mutation type " << mutationType << std::endl;
                break;
        }

        if (success) {
            std::cout << "Successfully applied mutation: " << mutation_type_to_string(mutationType) << std::endl;
        } else {
            std::cerr << "Warning: Mutation application failed for type " << mutation_type_to_string(mutationType) << std::endl;
        }

        return success;

    } catch (const std::exception& e) {
        std::cerr << "Exception in applyMutation: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <original.ll> <mutations.csv> <output.ll>" << std::endl;
        return 1;
    }

    const std::string originalFile = argv[1];
    const std::string csvFile = argv[2];
    const std::string outputFile = argv[3];
    const std::string tempFile = outputFile + ".tmp.ll";

    // Check if input files exist
    if (!std::filesystem::exists(csvFile)) {
        std::cerr << "Error: CSV file '" << csvFile << "' not found!" << std::endl;
        return 1;
    }

    if (!std::filesystem::exists(originalFile)) {
        std::cerr << "Error: Original file '" << originalFile << "' not found!" << std::endl;
        return 1;
    }
    
    // Copy original to temp file to start the process
    try {
        std::error_code ec;
        if (!std::filesystem::copy_file(originalFile, tempFile, std::filesystem::copy_options::overwrite_existing, ec)) {
            std::cerr << "Error: Failed to copy '" << originalFile << "' to '" << tempFile 
                      << "': " << ec.message() << std::endl;
            return 1;
        }
        std::cout << "Copied original file to temporary file: " << tempFile << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception while copying file: " << e.what() << std::endl;
        return 1;
    }
    
    std::ifstream file(csvFile);
    std::string line;
    
    // Skip header line
    std::getline(file, line);
    
    int stepCount = 0;
    std::cout << "Starting mutation application process..." << std::endl;
    
    // Process each line in the CSV
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = parseCSVLine(line);
        
        if (tokens.size() < 3) {
            std::cerr << "Warning: Skipping malformed line: " << line << std::endl;
            continue;
        }
        
        // Parse the mutation data
        int step = std::stoi(tokens[0]);
        std::string mutationTypeStr = tokens[1];
        std::string decisionsStr = tokens[2];
        
        MutationType mutationType = stringToMutationType(mutationTypeStr);
        std::vector<int> decisions = parseDecisions(decisionsStr);
        
        std::cout << "Step " << step << ": Applying " << mutationTypeStr 
                  << " with decisions: " << decisionsStr << std::endl;
        
        // Apply the mutation
        bool success = applyMutation(mutationType, decisions, tempFile, tempFile);
        
        if (!success) {
            std::cerr << "Failed to apply mutation at step " << step << std::endl;
            // Continue with next mutation instead of failing completely
        }
        
        stepCount++;
    }
    
    file.close();
    
    // Copy final result to output file
    try {
        std::error_code ec;
        if (!std::filesystem::copy_file(tempFile, outputFile, std::filesystem::copy_options::overwrite_existing, ec)) {
            std::cerr << "Error: Failed to copy final result from '" << tempFile << "' to '" << outputFile 
                      << "': " << ec.message() << std::endl;
            return 1;
        }
        std::cout << "Copied final result to: " << outputFile << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception while copying final result: " << e.what() << std::endl;
        return 1;
    }
    
    // Clean up temp file with proper error handling
    try {
        std::error_code ec;
        if (!std::filesystem::remove(tempFile, ec)) {
            std::cerr << "Warning: Failed to remove temporary file '" << tempFile 
                      << "': " << ec.message() << std::endl;
        } else {
            std::cout << "Cleaned up temporary file: " << tempFile << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception while removing temporary file '" << tempFile 
                  << "': " << e.what() << std::endl;
    }
    
    std::cout << "Process completed successfully!" << std::endl;
    std::cout << "Applied " << stepCount << " mutations" << std::endl;
    std::cout << "Final result written to: " << outputFile << std::endl;
    
    return 0;
}
