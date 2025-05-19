#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv) {
    const long N = 128;
    const long M = 128;
    const long J = 50;
    long sum = 0;

    for (long i = 1; i <= N; ++i) {
        for (long j=1;j<=M; ++j){
            for (long k=1;k<=J; ++k){
                int64_t mult = i * j * k;
                if (mult % 2 == 0) {
                    sum += mult;
                } else {
                    sum -= mult/2;
                }
            }
        }
    }

    printf("%lld\n", sum);
    return sum;
}
