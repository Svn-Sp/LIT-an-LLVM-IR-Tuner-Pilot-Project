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
#include "utils/file_management.cpp"
#include "utils/randomness_utils.cpp"
#include "output/output.cpp"
#include <iomanip>  
#include <nlohmann/json.hpp>

using json = nlohmann::json;


class BeamSearchTreeNode {
    public:
        BeamSearchTreeNode* parent;
        std::vector<BeamSearchTreeNode*> children;
        double avg_time;
        double std_dev_time;
        double result;
        int path_size;
        double run_score;
        double avg_path_run_score;
        MutationType mutationType;
        std::vector<int> decisions;
        BeamSearchTreeNode(){
            //root node constavg_path_run_scoreructor
            this->parent = nullptr;
            this->avg_time = 0.0;
            this->std_dev_time = 0.0;
            this->path_size = 0;
            this->avg_path_run_score = 1;
            this->run_score = 1;
        }
        BeamSearchTreeNode(BeamSearchTreeNode* parent, MutationType mutationType, std::vector<int> decisions,  double avg_time, double std_dev_time, double result, double og_runtime, double max_runtime){
            this->parent = parent;
            this->mutationType = mutationType;
            this->decisions = decisions;
            this->avg_time = avg_time;
            this->std_dev_time = std_dev_time;
            this->result = result;
            this->path_size = 1;
            this->run_score = this->calculate_run_score(og_runtime, max_runtime);
            this->avg_path_run_score = this->run_score;
            this->bubble_up_run_score(this->run_score);

        }
        void add_child(MutationType mutationType, std::vector<int> decisions,  double avg_time, double std_dev_time, double result, double og_runtime, double max_runtime){
            BeamSearchTreeNode* child = new BeamSearchTreeNode(this, mutationType, decisions, avg_time, std_dev_time, result, og_runtime, max_runtime);
            if (child) {
                children.push_back(child);
            } else {
                llvm::outs() << "Warning: Failed to create child node\n";
            }
        }
        float calculate_run_score(float og_runtime, float max_runtime) {
            float peak = og_runtime / 2; //Assume that the highest speedup we can reasonably expect is 2x
            if (this->result == 0) {
                return 0.8 + 0.2*std::clamp((max_runtime - this->avg_time) / (max_runtime - peak), 0.0, 1.0);
            }else{
                double unclamped_score;
                if (this->avg_time < peak){
                    float min_rt = og_runtime/5;
                    unclamped_score = (this->avg_time - min_rt) / (peak - min_rt);
                }else{
                    unclamped_score = (max_runtime - this->avg_time) / (max_runtime - peak);
                }
                return 0.2*std::clamp(unclamped_score, 0.0, 1.0);
            }
        }
        void bubble_up_run_score(float new_run_score){
            if (this->parent == nullptr){
                return;
            }
            this->parent->path_size++;
            float old_avg_path_run_score = this->parent->avg_path_run_score;
            this->parent->avg_path_run_score = old_avg_path_run_score * ((float)(this->parent->path_size-1) / (float)this->parent->path_size) + new_run_score * (1.0f / (float)this->parent->path_size);
            // Build moving average
            this->parent->bubble_up_run_score(new_run_score);
        }
        float calculate_score() const {
            //formular: score(v)=0.8*max(avg_path_run_score, run_score) + 0.2*(1-size of path compared to siblings)
            int total_path_size = this->parent->path_size - 1;
            int path_size = this->path_size;
            float relative_size = (float)path_size / (float)total_path_size;
            return 0.8*std::max(this->avg_path_run_score, this->run_score) + 0.2*(1-relative_size);
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

std::string safe_double_to_json(double value) {
    if (std::isinf(value)) {
        return value > 0 ? "\"Infinity\"" : "\"-Infinity\"";
    } else if (std::isnan(value)) {
        return "\"NaN\"";
    } else {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6) << value;
        return ss.str();
    }
}

std::string node_to_json(const BeamSearchTreeNode* node, int depth = 0) {
    std::stringstream ss;
    std::string indent(depth * 2, ' ');
    
    ss << indent << "{\n";
    
    // Only include mutation info if this is not the root node
    if (node->parent != nullptr) {
        ss << indent << "  \"avg_path_run_score\": " << safe_double_to_json(node->avg_path_run_score) << ",\n";
        ss << indent << "  \"path_size\": " << node->path_size << ",\n";
        ss << indent << "  \"score\": " << safe_double_to_json(node->calculate_score()) << ",\n"; //Remove for speed up
        ss << indent << "  \"mutationType\": \"" << mutation_type_to_string(node->mutationType) << "\",\n";
        ss << indent << "  \"decisions\": " << vector_to_json_array(node->decisions) << ",\n";
        ss << indent << "  \"avg_time\": " << safe_double_to_json(node->avg_time) << ",\n";
        ss << indent << "  \"std_dev_time\": " << safe_double_to_json(node->std_dev_time) << ",\n";
        ss << indent << "  \"result\": " << safe_double_to_json(node->result) << ",\n";
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

int write_tree_to_json(std::string file_name, BeamSearchTreeNode* root) {
    try {
        std::ofstream file(file_name);
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
void beam_search(std::string program_file, std::string modified_file, std::string output_file, OutputBase& correct_result, std::vector<Run>& runs){
    std::string sanitized_program_file = sanitizeFileName(program_file);
    std::string csv_file_name = "beam_search_results_" + sanitized_program_file + ".csv";
    std::string tree_file_name = "beam_search_tree_" + sanitized_program_file + ".json";

    BeamSearchTreeNode* root = new BeamSearchTreeNode();
    Run run;
    measure_time(program_file, output_file, correct_result, run);
    float og_runtime = run.avgDuration;
    float max_runtime = og_runtime * 1.25; // Assume max runtime for now
    runs.push_back(run);

    llvm::outs() << "Starting beam search\n";
    int evaluations = 0;
    std::vector<BeamSearchTreeNode*> children;
    std::vector<double> scores;
    while(true){
        llvm::outs() << "Copying original to modified\n";
        copyOriginalToModified(program_file, modified_file);
        Run run;
        BeamSearchTreeNode* selectedNode = root;
        int current_depth = 0;

        children = root->children;
        scores.clear();
        for (auto child : children){
            scores.push_back(child->calculate_score());
        }
        llvm::outs() << "Reapplying mutations:\n";
        while(children.size() > 0 && current_depth < MAX_MUTATIONS){
            current_depth++;
            scores.push_back(0.1); //Weight of adding new node at this level
            std::discrete_distribution<> distribution(scores.begin(), scores.end());
            int index = distribution(gen);
            if(index==scores.size()-1){
                llvm::outs() << "Create new child\n";
                children = {};
                scores = {};
                break;
            }
            selectedNode = selectedNode->children[index];
            reapplyMutation(run, selectedNode->mutationType, selectedNode->decisions, modified_file);
            scores.clear();
            for (auto child : selectedNode->children){
                scores.push_back(child->calculate_score());
            }
            children=selectedNode->children;
        }
        llvm::outs() << "Applying random mutation:\n";
        MutationType mutationType;
        std::vector<int> decisions;
        while(true){
            std::tie(mutationType, decisions) = applyRandomMutation(run, modified_file);
            if (decisions.size() > 0){
                break;
            }
        }
        int success = measure_time(modified_file, output_file, correct_result, run);
        if (run.avgDuration > max_runtime){
            max_runtime = run.avgDuration;
        }
        evaluations++;
        runs.push_back(run);

        if(success == 1){
            llvm::outs() << "Mutation successful\n";
            selectedNode->add_child(mutationType, decisions, run.avgDuration, run.stddevDuration, run.result, og_runtime, max_runtime);
        } else {
            llvm::outs() << "Mutation failed\n";
        }
        llvm::outs() << "Writing results to CSV and JSON\n";
        writeResultsToCSV(csv_file_name, runs);
        write_tree_to_json(tree_file_name, root);
        if(evaluations >= EVALUATIONS_BUDGET){
            return;
        }
    }
}