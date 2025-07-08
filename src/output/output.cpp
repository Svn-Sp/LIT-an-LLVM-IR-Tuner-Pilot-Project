#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>
#include <any>
#include <fstream>

class OutputBase{
    public:
        std::string output_file;
        OutputBase(){};
        virtual double get_distance(OutputBase& other) = 0;
};

template<typename T>
class Output : public OutputBase {
    public:
        T content;
        Output(std::string output_file){
            this->output_file = output_file;
        };
        virtual T read_output(std::string output_file) {
            throw std::runtime_error("Not implemented");
        }
};

#endif