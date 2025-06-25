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
    
    // Convert vector to array for the constructors
    int* decisionsArray = nullptr;
    try {
        decisionsArray = new int[decisions.size()];
        for (size_t i = 0; i < decisions.size(); i++) {
            decisionsArray[i] = decisions[i];
        }
        
        std::vector<int> result_decisions;
        bool success = false;
        
        switch (mutationType) {
            case ADD_RANDOM_ARITHMETIC: {
                AddRandomArithmetic addRandomArithmeticCustom(decisionsArray);
                result_decisions = addRandomArithmeticCustom.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            case REPLACE_ARITHMETIC: {
                ReplaceArithmetic replaceArithmeticCustom(decisionsArray);
                result_decisions = replaceArithmeticCustom.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            case MOVE_BLOCKWISE: {
                MoveBlockwise moveBlockwiseCustom(decisionsArray);
                result_decisions = moveBlockwiseCustom.run(inputFile.c_str(), outputFile.c_str());
                success = !result_decisions.empty();
                break;
            }
            case ADD_NEW_COND: {
                AddNewCond addNewCondCustom(decisionsArray);
                result_decisions = addNewCondCustom.run(inputFile.c_str(), outputFile.c_str());
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
        
        delete[] decisionsArray;
        return success;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in applyMutation: " << e.what() << std::endl;
        if (decisionsArray) delete[] decisionsArray;
        return false;
    }
}

int main() {
    const std::string csvFile = "fastest_correct_mutations.csv";
    const std::string originalFile = "original.ll";
    const std::string outputFile = "best.ll";
    const std::string tempFile = "temp.ll";
    
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
    std::filesystem::copy_file(originalFile, tempFile, std::filesystem::copy_options::overwrite_existing);
    
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
    std::filesystem::copy_file(tempFile, outputFile, std::filesystem::copy_options::overwrite_existing);
    
    // Clean up temp file
    std::filesystem::remove(tempFile);
    
    std::cout << "Process completed successfully!" << std::endl;
    std::cout << "Applied " << stepCount << " mutations" << std::endl;
    std::cout << "Final result written to: " << outputFile << std::endl;
    
    return 0;
}
