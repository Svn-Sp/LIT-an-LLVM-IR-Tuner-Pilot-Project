#include "tuning_strategies/random_tuning.cpp"
#include "tuning_strategies/genetic_tuning.cpp"
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char** argv) {
    // random_tuning();
    genetic_tuning();
}