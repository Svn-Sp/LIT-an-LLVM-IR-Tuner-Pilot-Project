#include "tuning_strategies/random_tuning.cpp"
#include "tuning_strategies/genetic_tuning.cpp"
#include "tuning_strategies/beam_search.cpp"
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include "output/output.cpp"
#include "output/scalar.cpp"

int main(int argc, char** argv) {
    std::string file_to_tune = argv[1];
    std::string modified_file = argv[2];
    std::string output_file = argv[3];
    std::string correct_result_file = argv[4];
    std::string output_type = argv[5];
    std::unique_ptr<OutputBase> correct_result;
    if (output_type == "scalar") {
        correct_result = std::make_unique<Scalar>(correct_result_file);
    }
    std::vector<Run> runs;
    beam_search(file_to_tune, modified_file, output_file, *correct_result, runs);
    // random_tuning();
    // genetic_tuning();
}