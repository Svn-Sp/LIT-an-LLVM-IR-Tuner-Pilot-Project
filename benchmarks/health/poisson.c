#include "health.h"

float my_rand(long long idum) {
    long long k;
    float answer;

    idum ^= MASK;
    k = idum / IQ;
    idum = IA * (idum - k * IQ) - IR * k;
    idum ^= MASK;
    if (idum < 0) {
        idum += IM;
    }
    answer = AM * idum;
    return answer;
}
