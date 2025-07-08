#pragma once

#include <string>
#include <memory>
#include "output.cpp"
#include <fstream>

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
            double res = std::abs(this->content - other_scalar->content);
            return res;
        }
        float read_output(std::string output_file) override {
            std::ifstream file(output_file);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open file: " + output_file);
            }
            float value;
            file >> value;
            if (file.fail()) {
                throw std::runtime_error("Failed to read float value from file: " + output_file);
            }
            file.close();
            return value;
        }
};
