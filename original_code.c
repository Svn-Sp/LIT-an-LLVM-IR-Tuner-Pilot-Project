#include <stdio.h>
#include <stdint.h>

// int main(int argc, char** argv) {
//     const int N = 600;
//     double A[N][N], B[N][N], C[N][N];
    
//     // Initialize matrices
//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < N; j++) {
//             A[i][j] = (double)(i + j) / N;
//             B[i][j] = (double)(i * j) / N;
//             C[i][j] = 0.0;
//         }
//     }
    
//     // Perform matrix multiplication
//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < N; j++) {
//             for (int k = 0; k < N; k++) {
//                 C[i][j] += A[i][k] * B[k][j];
//             }
//         }
//     }
    
//     // Print a sample value to verify
//     printf("%f\n", C[100][100]);
//     return 0;
// }
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
