#include <libgen.h>
#include <stdio.h>
#include <string.h>

#define PERM_ARRAY_SIZE 8
#define RUNS 100

static int permarray[PERM_ARRAY_SIZE];
static unsigned int pctr;

static void swap_int(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

static void initialize_perm(void) {
    for (int i = 1; i <= 7; i++) {
        permarray[i] = i - 1;
    }
}

__attribute__((section("OPTIMIZABLE_SECTION")))
static void permute(int n) {
    pctr += 1;
    if (n != 1) {
        permute(n - 1);
        for (int k = n - 1; k >= 1; k--) {
            swap_int(&permarray[n], &permarray[k]);
            permute(n - 1);
            swap_int(&permarray[n], &permarray[k]);
        }
    }
}

__attribute__((section("OPTIMIZABLE_SECTION")))
static unsigned int run_perm_kernel(void) {
    pctr = 0;
    for (int i = 0; i < 5; i++) {
        initialize_perm();
        permute(7);
    }
    return pctr;
}

int main(void) {
    unsigned long long checksum = 0;
    for (int i = 0; i < RUNS; i++) {
        checksum += run_perm_kernel();
    }

    char filepath[512];
    strcpy(filepath, __FILE__);
    char *dir = dirname(filepath);
    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s/output.txt", dir);

    FILE *file = fopen(output_path, "w");
    if (!file) {
        return 1;
    }
    fprintf(file, "%.0f\n", (double)checksum);
    fclose(file);

    printf("%.0f\n", (double)checksum);
    return 0;
}
