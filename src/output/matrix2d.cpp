

#pragma once

#include <string>
#include <memory>
#include "output.cpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Matrix2DOutput : public Output<std::vector<std::vector<double>>> {
    public:
        Matrix2DOutput(std::string output_file) : Output<std::vector<std::vector<double>>>(output_file) {
            this->content = this->read_output(output_file);
        }
        double get_distance(OutputBase& other) override {
            const Matrix2DOutput* other_matrix = dynamic_cast<const Matrix2DOutput*>(&other);
            if (!other_matrix) {
                throw std::runtime_error("Cannot compare Matrix2DOutput with different output type");
            }
            int max_rows = std::max(this->content.size(), other_matrix->content.size());
            int max_cols = std::max(this->content[0].size(), other_matrix->content[0].size());
            double sum = 0;
            for (int i = 0; i < max_rows; i++) {
                for (int j = 0; j < max_cols; j++) {
                    double val1 = (i < this->content.size() && j < this->content[i].size()) ? this->content[i][j] : 0.0;
                    double val2 = (i < other_matrix->content.size() && j < other_matrix->content[i].size()) ? other_matrix->content[i][j] : 0.0;
                    sum += std::abs(val1 - val2);
                }
            }
            return sum;
        }
        std::vector<std::vector<double>> read_output(std::string output_file) override {
            std::ifstream file(output_file);
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            size_t pos = 0;
            while ((pos = content.find("-nan", pos)) != std::string::npos) {
                content.replace(pos, 4, "0");
                pos += 4;
            }
            while ((pos = content.find("null", pos)) != std::string::npos) {
                content.replace(pos, 4, "0.0");
                pos += 4;
            }
            json data = json::parse(content);
            return data.get<std::vector<std::vector<double>>>();
        }
};
