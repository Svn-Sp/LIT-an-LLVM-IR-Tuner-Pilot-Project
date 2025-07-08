#ifndef RUN_H
#define RUN_H

#include <vector>
#include <tuple>
#include <string>
#include "mutations/mutation.h"

class Run {
public:
    std::vector<std::tuple<MutationType, std::vector<int>>> mutations;
    bool success;
    double avgDuration;
    double stddevDuration;
    double result;
    
    std::string asString();
};

#endif // RUN_H