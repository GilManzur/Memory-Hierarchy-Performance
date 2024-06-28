#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

void remove_substring(char* str, const char* start, const char* end) {
    char* p1 = strstr(str, start);
    if (p1 != NULL) {
        char* p2 = strstr(p1, end);
        if (p2 != NULL) {
            p2 += strlen(end);
            memmove(p1, p2, strlen(p2) + 1);
        }
    }
}

int process_line(char* line, char* output) {
    char arg1[50], arg2[50], arg3[50], arg4[50], arg5[50], arg6[50];
    sscanf(line, "%s %s %s %s %s %s", arg1, arg2, arg3, arg4, arg5, arg6);

    // Remove characters from "(" to ":" in the 3rd argument
    remove_substring(arg3, "(", ":");

    // Check if the 5th argument contains "sw" or "lw"
    if (strstr(arg5, "sw") != NULL || strstr(arg5, "lw") != NULL) {
        sprintf(output, "%s %s %s", arg3, arg5, arg6);
        return 1; // Indicate that the line should be kept
    }
    return 0; // Indicate that the line should be discarded
}

int main() {
    FILE *inputFile, *outputFile;
    char line[MAX_LINE_LENGTH];
    char processedLine[MAX_LINE_LENGTH];
    
    inputFile = fopen("linpack_val.txt", "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Could not open input file\n");
        return 1;
    }
    
    outputFile = fopen("linpack_val_final.txt", "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Could not open output file\n");
        fclose(inputFile);
        return 1;
    }
    
    while (fgets(line, sizeof(line), inputFile)) {
        if (process_line(line, processedLine)) {
            fprintf(outputFile, "%s\n", processedLine);
        }
    }
    
    fclose(inputFile);
    fclose(outputFile);
    
    return 0;
}
