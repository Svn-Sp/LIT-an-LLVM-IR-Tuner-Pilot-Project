#ifndef CORE_CPP
#define CORE_CPP

#include <vector>
#include <tuple>
#include <sstream>
#include <cmath>
#include "mutation.cpp"

class Run {
    public:
    std::vector<std::tuple<MutationType, std::vector<int>>> mutations;
    double avgDuration;
    double stddevDuration;
    double result;
    std::string asString(){
        std::stringstream ss;
        ss << "Run: " << avgDuration << " " << stddevDuration << "\n";
        for (std::tuple<MutationType, std::vector<int>> mutation : mutations) {
            ss << std::get<0>(mutation) << " ";
            for(int i : std::get<1>(mutation)){
                ss << i << " ";
            }
            ss << "\n";
        }
        return ss.str();
    }
    
};

#endif // CORE_CPP