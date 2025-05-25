#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tarcode.h"

void generateTargetCode(const char *irFilename, const char *targetFilename) {
    FILE *irFile = fopen(irFilename, "r");
    FILE *targetFile = fopen(targetFilename, "w");

    if (!irFile || !targetFile) {
        perror("Error opening file");
        return;
    }

    char line[256];
    int labelCounter = 0;

    //fprintf(targetFile, ".data\n\n.text\n.global _start\n_start:\n");

    while (fgets(line, sizeof(line), irFile)) {
        char lhs[64], rhs[64], op[8], arg1[64], arg2[64], arg3[64];

        if (strlen(line) <= 1 || line[0] == '#') continue;
        line[strcspn(line, "\n")] = 0;

        if (strchr(line, ':')) {
            fprintf(targetFile, "%s\n", line);
        }
        else if (strncmp(line, "return", 6) == 0) {
            sscanf(line, "return %s", rhs);
            fprintf(targetFile, "    MOV R0, %s\n", rhs);
            fprintf(targetFile, "    JMP exit_program\n");
        }
        else if (strncmp(line, "goto", 4) == 0) {
            sscanf(line, "goto %s", rhs);
            fprintf(targetFile, "    JMP %s\n", rhs);
        }
        else if (strncmp(line, "if", 2) == 0) {
            if (sscanf(line, "if %s < %s goto %s", arg1, arg2, arg3) == 3) {
                fprintf(targetFile, "    CMP %s, %s\n", arg1, arg2);
                fprintf(targetFile, "    JL %s\n", arg3);
            }
            else if (sscanf(line, "if %s <= %s goto %s", arg1, arg2, arg3) == 3) {
                fprintf(targetFile, "    CMP %s, %s\n", arg1, arg2);
                fprintf(targetFile, "    JLE %s\n", arg3);
            }
            else if (sscanf(line, "if %s > %s goto %s", arg1, arg2, arg3) == 3) {
                fprintf(targetFile, "    CMP %s, %s\n", arg1, arg2);
                fprintf(targetFile, "    JG %s\n", arg3);
            }
            else if (sscanf(line, "if %s >= %s goto %s", arg1, arg2, arg3) == 3) {
                fprintf(targetFile, "    CMP %s, %s\n", arg1, arg2);
                fprintf(targetFile, "    JGE %s\n", arg3);
            }
            else if (sscanf(line, "if %s == %s goto %s", arg1, arg2, arg3) == 3) {
                fprintf(targetFile, "    CMP %s, %s\n", arg1, arg2);
                fprintf(targetFile, "    JE %s\n", arg3);
            }
            else if (sscanf(line, "if %s goto %s", arg1, arg2) == 2) {
                fprintf(targetFile, "    CMP %s, 0\n", arg1);
                fprintf(targetFile, "    JNZ %s\n", arg2);
            }
        }
        else if (sscanf(line, "%[^=]= %s %s %s", lhs, arg1, op, arg2) == 4) {
            if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || 
                strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
                fprintf(targetFile, "    MOV R1, %s\n", arg1);
                if (strcmp(op, "+") == 0)
                    fprintf(targetFile, "    ADD R1, %s\n", arg2);
                else if (strcmp(op, "-") == 0)
                    fprintf(targetFile, "    SUB R1, %s\n", arg2);
                else if (strcmp(op, "*") == 0)
                    fprintf(targetFile, "    MUL R1, %s\n", arg2);
                else if (strcmp(op, "/") == 0)
                    fprintf(targetFile, "    DIV R1, %s\n", arg2);
                fprintf(targetFile, "    MOV %s, R1\n", lhs);
            }
            else if (strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
                     strcmp(op, ">") == 0 || strcmp(op, ">=") == 0 ||
                     strcmp(op, "==") == 0) {
                char trueLabel[32], endLabel[32];
                sprintf(trueLabel, "true_%d", labelCounter);
                sprintf(endLabel, "end_%d", labelCounter);
                labelCounter++;

                fprintf(targetFile, "    CMP %s, %s\n", arg1, arg2);
                fprintf(targetFile, "    MOV %s, 0\n", lhs);

                if (strcmp(op, "<") == 0)
                    fprintf(targetFile, "    JL %s\n", trueLabel);
                else if (strcmp(op, "<=") == 0)
                    fprintf(targetFile, "    JLE %s\n", trueLabel);
                else if (strcmp(op, ">") == 0)
                    fprintf(targetFile, "    JG %s\n", trueLabel);
                else if (strcmp(op, ">=") == 0)
                    fprintf(targetFile, "    JGE %s\n", trueLabel);
                else if (strcmp(op, "==") == 0)
                    fprintf(targetFile, "    JE %s\n", trueLabel);

                fprintf(targetFile, "    JMP %s\n", endLabel);
                fprintf(targetFile, "%s:\n", trueLabel);
                fprintf(targetFile, "    MOV %s, 1\n", lhs);
                fprintf(targetFile, "%s:\n", endLabel);
            }
        }
        else if (sscanf(line, "%[^=]= %s", lhs, rhs) == 2) {
            fprintf(targetFile, "    MOV %s, %s\n", lhs, rhs);
        }
    }

    fprintf(targetFile, "exit_program:\n");
    fprintf(targetFile, "    HALT\n");

    fclose(irFile);
    fclose(targetFile);
    printf("Target code written to %s\n", targetFilename);
}