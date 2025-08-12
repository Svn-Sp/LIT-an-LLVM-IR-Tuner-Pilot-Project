#include "json.hpp"
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <libgen.h>
#include <vector>

using json = nlohmann::json;

#define N 512
#define M 512

__attribute__((section("OPTIMIZABLE_SECTION")))
void computation(const std::vector<std::vector<double>>& matrix1, 
                const std::vector<std::vector<double>>& matrix2, 
                double* result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            result[i * M + j] = matrix1[i][j] * matrix2[i][j];
        }
    }
}

void read_matrix(const char* filename, std::vector<std::vector<double>>& matrix) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return;
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *json_data = (char *)malloc(fsize + 1);
    fread(json_data, 1, fsize, fp);
    json_data[fsize] = '\0';
    fclose(fp);
    json data = json::parse(json_data);
    free(json_data);
    matrix = data.get<std::vector<std::vector<double>>>();
}
int main() {
    double* result = (double*)malloc(N * M * sizeof(double));
    if (!result) {
        std::cerr << "Error: Memory allocation failed for result" << std::endl;
        return 1;
    }
    std::vector<std::vector<double>> matrix;
    std::vector<std::vector<double>> matrix2;
    char filepath[256];
    strcpy(filepath, __FILE__);
    char *dir = dirname(filepath);
    char input1_path[512], input2_path[512];
    sprintf(input1_path, "%s/input1.json", dir);
    sprintf(input2_path, "%s/input2.json", dir);
    read_matrix(input1_path, matrix);
    read_matrix(input2_path, matrix2);
    computation(matrix, matrix2, result);

    // Create output JSON array with proper dimensions
    json output = json::array();
    for (int i = 0; i < N; i++) {
        json row = json::array();
        for (int j = 0; j < M; j++) {
            row.push_back(result[i * M + j]);
        }
        output.push_back(row);
    }

    char output_path[512];
    sprintf(output_path, "%s/output.json", dir);
    FILE *fp3 = fopen(output_path, "w");
    if (!fp3) {
        std::cerr << "Error: Could not open output.json for writing" << std::endl;
        free(result);
        return 1;
    }
    
    std::string output_str = output.dump();
    fwrite(output_str.c_str(), 1, output_str.size(), fp3);
    fclose(fp3);
    
    free(result);  // Use free instead of delete[] for malloc'd memory
    return 0;
}