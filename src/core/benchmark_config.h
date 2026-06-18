#ifndef BENCHMARK_CONFIG_H
#define BENCHMARK_CONFIG_H

#include <memory>
#include <string>
#include <vector>
#include "output/output.cpp"
#include "nlohmann/json.hpp"

struct InputCase {
    std::string name;
    std::vector<std::string> input_paths;
    std::string reference_output_path;
};

struct BenchmarkEval {
    bool accepts_input = false;
    std::string output_file;
    std::string output_type;
    std::unique_ptr<OutputBase> correct_result;
    std::vector<InputCase> input_cases;
};

std::unique_ptr<OutputBase> make_correct_output(const std::string& output_type,
                                                const std::string& path);

BenchmarkEval load_benchmark_eval(const std::string& config_path,
                                  const nlohmann::json& config);

#endif // BENCHMARK_CONFIG_H
