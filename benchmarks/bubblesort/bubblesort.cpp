#include <stdio.h>
#include <stdlib.h>
#include "json.hpp"
#include <math.h>
#include <string.h>
#include <libgen.h>

using json = nlohmann::json;

__attribute__((section("OPTIMIZABLE_SECTION")))
void Bubble(int* unsorted, int size) {
	int i, j;
	int top = size;
	
	while ( top>1 ) {
		
		i=0;
		while ( i<top-1 ) {
			
			if ( unsorted[i] > unsorted[i+1] ) {
				j = unsorted[i];
				unsorted[i] = unsorted[i+1];
				unsorted[i+1] = j;
			}
			i=i+1;
		}
		
		top=top-1;
	}
}

int main()
{
    // Try to open input.json in the current directory first
    FILE *fp = fopen("input.json", "r");
    if (!fp) {
        // If not found in current directory, try relative to the executable
        char filepath[256];
        strcpy(filepath, __FILE__);
        char *dir = dirname(filepath);
        char output_path[512];
        sprintf(output_path, "%s/input.json", dir);
        fp = fopen(output_path, "r");
        if (!fp) {
            fprintf(stderr, "Could not open input.json in current directory or %s\n", output_path);
            return 1;
        }
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *json_data = (char *)malloc(fsize + 1);
    if (!json_data) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        return 1;
    }
    fread(json_data, 1, fsize, fp);
    json_data[fsize] = 0;
    fclose(fp);

    json j = json::parse(json_data);
    free(json_data);

    int size = j.size();
    int* unsorted = (int*)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        unsorted[i] = j[i];
    }
    Bubble(unsorted, size);
    
    // Always write output.json relative to the source code location
    char filepath[256];
    strcpy(filepath, __FILE__);
    char *dir = dirname(filepath);
    char output_path[512];
    sprintf(output_path, "%s/output.json", dir);
    FILE *out_fp = fopen(output_path, "w");
    if (!out_fp) {
        fprintf(stderr, "Could not open %s for writing\n", output_path);
        free(unsorted);
        return 1;
    }
    fprintf(out_fp, "[");
    for (int i = 0; i < size; i++) {
        fprintf(out_fp, "%d", unsorted[i]);
        if (i != size - 1) {
            fprintf(out_fp, ",");
        }
    }
    fprintf(out_fp, "]\n");
    fclose(out_fp);
    free(unsorted);
}