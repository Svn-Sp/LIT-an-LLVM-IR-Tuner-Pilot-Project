#include "core/benchmark_config.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

#include "nlohmann/json.hpp"
#include "output/scalar.cpp"
#include "output/array.cpp"
#include "output/matrix2d.cpp"

std::unique_ptr<OutputBase> make_correct_output(const std::string& output_type,
                                                const std::string& path) {
    if (output_type == "scalar") {
        return std::make_unique<Scalar>(path);
    }
    if (output_type == "array") {
        return std::make_unique<ArrayOutput>(path);
    }
    if (output_type == "matrix2d") {
        return std::make_unique<Matrix2DOutput>(path);
    }
    throw std::runtime_error("Unknown output_type: " + output_type);
}

BenchmarkEval load_benchmark_eval(const std::string& config_path,
                                  const nlohmann::json& config) {
    BenchmarkEval eval;
    eval.output_file = config.at("output_file").get<std::string>();
    eval.output_type = config.at("output_type").get<std::string>();
    eval.accepts_input = config.value("accepts_input", false);

    if (!eval.accepts_input) {
        eval.correct_result = make_correct_output(eval.output_type,
                                                  config.at("correct_output").get<std::string>());
        return eval;
    }

    if (!config.contains("input_files") || config["input_files"].empty()) {
        throw std::runtime_error("accepts_input is true but input_files is missing or empty");
    }

    std::vector<std::string> input_files;
    for (const auto& file : config["input_files"]) {
        input_files.push_back(file.get<std::string>());
    }

    std::filesystem::path benchmark_dir = std::filesystem::path(config_path).parent_path();
    std::filesystem::path input_root = benchmark_dir / "input";
    std::string reference_filename = std::filesystem::path(config.at("correct_output").get<std::string>()).filename().string();

    if (!std::filesystem::exists(input_root)) {
        throw std::runtime_error("input directory not found: " + input_root.string());
    }

    std::vector<std::filesystem::path> case_dirs;
    for (const auto& entry : std::filesystem::directory_iterator(input_root)) {
        if (entry.is_directory()) {
            case_dirs.push_back(entry.path());
        }
    }

    if (case_dirs.empty()) {
        throw std::runtime_error("no input cases found in " + input_root.string());
    }

    for (const auto& case_dir : case_dirs) {
        InputCase input_case;
        input_case.name = case_dir.filename().string();

        for (const auto& input_file : input_files) {
            std::filesystem::path input_path = case_dir / input_file;
            if (!std::filesystem::exists(input_path)) {
                throw std::runtime_error("missing input file: " + input_path.string());
            }
            input_case.input_paths.push_back(std::filesystem::absolute(input_path).string());
        }

        std::filesystem::path reference_path = case_dir / reference_filename;
        if (!std::filesystem::exists(reference_path)) {
            throw std::runtime_error("missing reference output: " + reference_path.string());
        }
        input_case.reference_output_path = std::filesystem::absolute(reference_path).string();
        eval.input_cases.push_back(std::move(input_case));
    }

    return eval;
}
