#ifndef RUN_CPP
#define RUN_CPP

#include "core/run.h"
#include <sstream>
#include <cmath>
#include <sqlite3.h>
#include "constants.h"
#include <chrono>

std::string Run::asString() {
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


#endif // RUN_CPP