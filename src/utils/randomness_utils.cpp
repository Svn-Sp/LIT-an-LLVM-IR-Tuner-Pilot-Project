#ifndef RANDOMNESS_UTILS_CPP
#define RANDOMNESS_UTILS_CPP

#include <random>

std::random_device rd;
std::mt19937 gen(rd());

#endif