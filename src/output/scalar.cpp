#pragma once

#include <string>
#include <memory>
#include "output.cpp"
#include <fstream>
#include <limits>
#include <cmath>

class Scalar : public Output<float> {
    public:
        Scalar(std::string output_file) : Output<float>(output_file) {
            this->content = this->read_output(output_file);
        }
        double get_distance(OutputBase& other) override {
            const Scalar* other_scalar = dynamic_cast<const Scalar*>(&other);
            if (!other_scalar) {
                throw std::runtime_error("Cannot compare Scalar with different output type");
            }
            return std::abs(this->content - other_scalar->content);
        }
        float read_output(std::string output_file) override {
            std::ifstream file(output_file);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open file: " + output_file);
            }
            std::string content;
            file >> content;
            if (file.fail()) {
                throw std::runtime_error("Failed to read from file: " + output_file);
            }
            file.close();
            float val = std::stof(content);
            if (std::isnan(val) || std::isinf(val)) {
                throw std::runtime_error("NaN/inf in scalar output: " + output_file);
            }
            return val;
        }
};
