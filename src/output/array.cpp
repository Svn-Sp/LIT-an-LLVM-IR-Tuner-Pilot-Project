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


class ArrayOutput : public Output<std::vector<float>> {
    public:
        ArrayOutput(std::string output_file) : Output<std::vector<float>>(output_file) {
            this->content = this->read_output(output_file);
        }
        double get_distance(OutputBase& other) override {
            const ArrayOutput* other_array = dynamic_cast<const ArrayOutput*>(&other);
            if (!other_array) {
                throw std::runtime_error("Cannot compare ArrayOutput with different output type");
            }
            llvm::outs() << "First element of this: " << this->content[0] << "\n";
            llvm::outs() << "First element of other: " << other_array->content[0] << "\n";
            int max_length = std::max(this->content.size(), other_array->content.size());
            double sum = 0;
            for (int i = 0; i < max_length; i++) {
                double val1 = (i < this->content.size()) ? this->content[i] : 0.0;
                double val2 = (i < other_array->content.size()) ? other_array->content[i] : 0.0;
                sum += std::abs(val1 - val2);
            }
            llvm::outs() << "Sum: " << sum << "\n";
            return sum;
        }
        std::vector<float> read_output(std::string output_file) override {
            std::ifstream file(output_file);
            json json_data;
            file >> json_data;
            std::vector<float> result;
            for (const auto& value : json_data) {
                result.push_back(value.get<float>());
            }
            return result;
        }
};
