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
#include "mutations/move_blockwise.cpp"
#include "mutations/mutation.h"
#include "mutations/replace_arithmetic.cpp"
#include "mutations/add_random_arithmetic.cpp"
#include <csetjmp>  // Added for setjmp/longjmp
#include <sstream>  // Add this for stringstream
#include "utils/file_management.cpp"
#include "core/run.cpp"
#include "constants.h"
#include "utils/randomness_utils.cpp"
#include "mutations/mutation_factory.cpp"
#include "utils/measure_time.cpp"
#include "mutations/mutation.h"
#include "utils/randomness_utils.cpp"

class BeamSearchTreeNode {
    public:
        BeamSearchTreeNode* parent;
        std::vector<BeamSearchTreeNode*> children;
        double avg_time;
        double std_dev_time;
        std::string result;
        double score;
        MutationType mutationType;
        std::vector<int> decisions;
        BeamSearchTreeNode(){
            //root node constructor
            this->parent = nullptr;
            this->score = 1.0;
            this->avg_time = 0.0;
            this->std_dev_time = 0.0;
        }
        BeamSearchTreeNode(BeamSearchTreeNode* parent, MutationType mutationType, std::vector<int> decisions, double score, double avg_time, double std_dev_time, std::string result){
            this->parent = parent;
            this->mutationType = mutationType;
            this->decisions = decisions;
            this->score = score;
            this->avg_time = avg_time;
            this->std_dev_time = std_dev_time;
            this->result = result;
        }
        void addChild(MutationType mutationType, std::vector<int> decisions, double score, double avg_time, double std_dev_time, std::string result){
            BeamSearchTreeNode* child = new BeamSearchTreeNode(this, mutationType, decisions, score, avg_time, std_dev_time, result);
            if (child) {
                children.push_back(child);
            } else {
                llvm::outs() << "Warning: Failed to create child node\n";
            }
        }
};

std::string vector_to_json_array(const std::vector<int>& vec) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) ss << ",";
        ss << vec[i];
    }
    ss << "]";
    return ss.str();
}

std::string node_to_json(const BeamSearchTreeNode* node, int depth = 0) {
    std::stringstream ss;
    std::string indent(depth * 2, ' ');
    
    ss << indent << "{\n";
    ss << indent << "  \"score\": " << node->score << ",\n";
    
    // Only include mutation info if this is not the root node
    if (node->parent != nullptr) {
        ss << indent << "  \"mutationType\": \"" << mutation_type_to_string(node->mutationType) << "\",\n";
        ss << indent << "  \"decisions\": " << vector_to_json_array(node->decisions) << ",\n";
        ss << indent << "  \"avg_time\": " << node->avg_time << ",\n";
        ss << indent << "  \"std_dev_time\": " << node->std_dev_time << ",\n";
        std::string clean_result = node->result;
        clean_result.erase(std::remove(clean_result.begin(), clean_result.end(), '\n'), clean_result.end());
        ss << indent << "  \"result\": \"" << clean_result << "\",\n";
    }
    
    ss << indent << "  \"children\": [\n";
    for (size_t i = 0; i < node->children.size(); ++i) {
        if (i > 0) ss << ",\n";
        ss << node_to_json(node->children[i], depth + 2);
    }
    ss << "\n" << indent << "  ]\n";
    ss << indent << "}";
    
    return ss.str();
}

int write_tree_to_json(BeamSearchTreeNode* root) {
    try {
        std::ofstream file("beam_search_tree.json");
        if (!file.is_open()) {
            llvm::outs() << "Error: Could not open file for writing\n";
            return -1;
        }
        
        std::string json_content = node_to_json(root);
        file << json_content;
        file.close();
        
        llvm::outs() << "Beam search tree written to beam_search_tree.json\n";
        return 0;
    } catch (const std::exception& e) {
        llvm::outs() << "Error writing tree to JSON: " << e.what() << "\n";
        return -1;
    }
}

void beam_search(){
    std::vector<std::tuple<double, double, std::string>> results;
    std::vector<Run> runs;
    std::string result;
    BeamSearchTreeNode* root = new BeamSearchTreeNode();
    std::random_device rd;
    std::mt19937 gen(rd());
    Run run;
    auto [og_avg_time, og_std_dev_time] = measure_time(result, run, results, "lli original.ll"); //Get original time

    int evaluations = 0;
    while(true){
        
        std::uniform_int_distribution<> dis(MIN_MUTATIONS, MAX_MUTATIONS);
        int depth = dis(gen);
        
        copyOriginalToModified(ORIGINAL_CODE, MODIFIED_CODE);
        Run run;
        BeamSearchTreeNode* selectedNode = root;
        int current_depth = 0;
        
        std::vector<BeamSearchTreeNode*> children = root->children;
        std::vector<double> scores = {};
        for (auto child : children){
            scores.push_back(child->score);
        }
        while(children.size() > 0 && current_depth < depth && std::any_of(scores.begin(), scores.end(), [](double score) { return score > 0; })){
            std::discrete_distribution<> distribution(scores.begin(), scores.end());
            int index = distribution(gen);
            selectedNode = selectedNode->children[index];
            reapplyMutation(run, selectedNode->mutationType, selectedNode->decisions);
            current_depth++;
            scores.clear();
            for (auto child : selectedNode->children){
                scores.push_back(child->score);
            }
            children=selectedNode->children;
        }
        
        auto [mutationType, decisions] = applyRandomMutation(run);
        auto [avg_time, std_dev_time] = measure_time(result, run, results);
        evaluations++;
        runs.push_back(run);
        double score;
        
        if(!result.empty()){
            llvm::outs() << "Mutation successful\n";
            score = og_avg_time / avg_time;
            if(decisions.size() > 0){
                selectedNode->addChild(mutationType, decisions, score, avg_time, std_dev_time, result);
            }
        } else {
            llvm::outs() << "Mutation failed\n";
            score = 0.0;
        }
        if (evaluations % 50 == 0){
            llvm::outs() << "Writing results to CSV and JSON\n";
            writeResultsToCSV("beam_search_results.csv", results);
            write_tree_to_json(root);
        }
        if(evaluations >= EVALUATIONS_BUDGET){
            writeResultsToCSV("beam_search_results.csv", results);
            return;
        }
    }  
    writeResultsToCSV("beam_search_results.csv", results);
}