#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "tarcode.h"
#define MAX_CODE_SIZE 10000

int main()
{
    FILE *file = fopen("input.txt", "r");
    if (!file)
    {
        perror("input.txt");
        return 1;
    }

    char code[MAX_CODE_SIZE];
    fread(code, 1, MAX_CODE_SIZE, file);
    fclose(file);

    tokenize(code);
    writeTokensToFile("tokens.txt");

    parseTokens(tokens, tokenCount);
    checkSemantics(tokens, tokenCount);

    generateIR(tokens, tokenCount);
    generateTargetCode("ir.txt", "target.txt");
    return 0;
}