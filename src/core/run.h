#ifndef RUN_H
#define RUN_H

#include <vector>
#include <tuple>
#include <string>
#include "mutations/mutation.h"

class Run {
public:
    std::vector<std::tuple<MutationType, std::vector<int>>> mutations;
    double avgDuration;
    double stddevDuration;
    double result;
    
    std::string asString();
    bool saveToDb();
};

#endif // RUN_H