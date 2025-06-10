#include <stdio.h>
#include <stdint.h>

int load_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    int64_t number;
    if (fscanf(file, "%lld", &number) != 1) {
        printf("Error reading number\n");
        fclose(file);
        return 1;
    }
    fclose(file);
    return number;
}

__attribute__((section("OPTIMIZABLE_SECTION"))) int main(int argc, char** argv) {
    int64_t number = load_file(argv[1]);
    number = number + 1;
    printf("%lld\n", number);
    return 0;
}

int test(){
    return 1;
}