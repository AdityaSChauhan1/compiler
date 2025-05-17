#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

#define MAX_SYMBOLS 100

typedef struct
{
    char name[64];
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

int isDeclared(const char *name)
{
    for (int i = 0; i < symbolCount; ++i)
        if (strcmp(symbolTable[i].name, name) == 0)
            return 1;
    return 0;
}

void declare(const char *name)
{
    if (isDeclared(name))
    {
        printf("Semantic Error: Redeclaration of variable '%s'\n", name);
        exit(1);
    }
    strcpy(symbolTable[symbolCount++].name, name);
}

void checkSemantics(Token tokens[], int count)
{
    symbolCount = 0;
    int inMainFunction = 0;
    int returnFound = 0;

    for (int i = 0; i < count; ++i)
    {
        // function start: int main(
        if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "int") == 0 &&
            tokens[i + 1].type == TOKEN_IDENTIFIER && strcmp(tokens[i + 1].value, "main") == 0)
        {
            inMainFunction = 1;
        }

        // variable declarations
        if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "int") == 0)
        {
            if (tokens[i + 1].type == TOKEN_IDENTIFIER &&
                (i == 0 || strcmp(tokens[i - 1].value, "return") != 0))
            {
                declare(tokens[i + 1].value);
            }
        }

        // Variable usage
        if (tokens[i].type == TOKEN_IDENTIFIER)
        {
            if (!isDeclared(tokens[i].value) &&
                strcmp(tokens[i].value, "main") != 0)
            {
                printf("Semantic Error: Undeclared variable '%s'\n", tokens[i].value);
                exit(1);
            }
        }

        // return statement
        if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "return") == 0)
        {
            returnFound = 1;

            if (tokens[i + 1].type != TOKEN_NUMBER && tokens[i + 1].type != TOKEN_IDENTIFIER)
            {
                printf("Semantic Error: Invalid return value\n");
                exit(1);
            }
        }

        // while and if conditions
        if ((tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "while") == 0) ||
            (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "if") == 0))
        {
            if (tokens[i + 1].type != TOKEN_LPAREN)
            {
                printf("Semantic Error: Missing '(' after '%s'\n", tokens[i].value);
                exit(1);
            }

            int j = i + 2;
            while (tokens[j].type != TOKEN_RPAREN && tokens[j].type != TOKEN_EOF)
            {
                if (tokens[j].type == TOKEN_IDENTIFIER && !isDeclared(tokens[j].value))
                {
                    printf("Semantic Error: Undeclared variable '%s' in condition\n", tokens[j].value);
                    exit(1);
                }
                j++;
            }

            if (tokens[j].type != TOKEN_RPAREN)
            {
                printf("Semantic Error: Missing ')' in condition\n");
                exit(1);
            }
        }
    }

    if (inMainFunction && !returnFound)
    {
        printf("Semantic Error: No return statement in main()\n");
        exit(1);
    }

    printf("Semantic analysis passed: All variables and returns are valid\n");
}