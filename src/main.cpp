#include "tuning_strategies/random_tuning.cpp"
#include "tuning_strategies/genetic_tuning.cpp"
#include "tuning_strategies/beam_search.cpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include "core/benchmark_config.h"
#include "nlohmann/json.hpp"

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
    BenchmarkEval eval = load_benchmark_eval(config_file, config);
    std::string results_file_prefix = argv[2];
    std::vector<Run> runs;
    beam_search(file_to_tune, modified_file, eval, runs, results_file_prefix);
    // random_tuning();
    // genetic_tuning();
}