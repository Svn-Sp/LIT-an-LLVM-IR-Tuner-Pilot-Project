#include "tuning_strategies/random_tuning.cpp"
#include "tuning_strategies/genetic_tuning.cpp"
#include "tuning_strategies/beam_search.cpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include "output/output.cpp"
#include "output/scalar.cpp"
#include "output/array.cpp"
#include "nlohmann/json.hpp"
#include "output/matrix2d.cpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file> <results_file>" << std::endl;
        return 1;
    }
    std::string config_file = argv[1];
    std::ifstream config_file_stream(config_file);
    nlohmann::json config;
    config_file_stream >> config;
    std::string file_to_tune = config["original"];
    std::string modified_file = config["modified"];
    std::string correct_result_file = config["correct_output"];
    std::string output_file = config["output_file"];
    std::string output_type = config["output_type"];
    std::unique_ptr<OutputBase> correct_result;
    if (output_type == "scalar") {
        correct_result = std::make_unique<Scalar>(correct_result_file);
    }
    else if (output_type == "array") {
        correct_result = std::make_unique<ArrayOutput>(correct_result_file);
    }
    else if (output_type == "matrix2d") {
        correct_result = std::make_unique<Matrix2DOutput>(correct_result_file);
    }
    std::string results_file_prefix = argv[2];
    std::vector<Run> runs;
    beam_search(file_to_tune, modified_file, output_file, *correct_result, runs, results_file_prefix);
    // random_tuning();
    // genetic_tuning();
}