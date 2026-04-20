#pragma once
#include <vector>
#include <random>
#include <iostream>
#include "llvm/Support/raw_ostream.h"

class DecisionMaker {
public:
    std::vector<int> decisions;
    bool new_decisions = true;
    int current_decision = 0;
    std::random_device rd;
    std::mt19937 gen;

    // New-decisions mode: generate and record decisions on the fly.
    DecisionMaker() : new_decisions(true), current_decision(0), gen(rd()) {}

    // Replay mode: play back a previously recorded decision sequence.
    DecisionMaker(std::vector<int> recorded)
        : decisions(std::move(recorded)), new_decisions(false),
          current_decision(0), gen(rd()) {}

    int make_decision(int min, int max) {
        if (new_decisions) {
            std::uniform_int_distribution<> dis(min, max);
            int decision = dis(gen);
            decisions.push_back(decision);
            return decision;
        }

        if (current_decision >= (int)decisions.size()) {
            std::cerr << "Warning: ran out of recorded decisions at index "
                      << current_decision << ", wrapping.\n";
            current_decision = 0;
        }
        return decisions[current_decision++];
    }

    void print_decisions() {
        llvm::outs() << "Decisions made: " << decisions.size() << "\n";
        llvm::outs() << "DECISIONS:[";
        for (int i = 0; i < (int)decisions.size(); i++) {
            llvm::outs() << decisions[i];
            if (i < (int)decisions.size() - 1) llvm::outs() << ",";
        }
        llvm::outs() << "]\n";
    }
};
