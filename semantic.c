#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "suggest.h"

#define MAX_SYMBOLS 100

typedef struct
{
    char name[64];
    char type[10];  // Store the type of the variable (int, float, etc.)
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

int getVariableType(const char *name)
{
    for (int i = 0; i < symbolCount; ++i)
        if (strcmp(symbolTable[i].name, name) == 0)
            return i;
    return -1;
}

void declare(const char *name, const char *type)
{
    if (isDeclared(name))
    {
        printf("Semantic Error: Redeclaration of variable '%s'\n", name);
        exit(1);
    }
    strcpy(symbolTable[symbolCount].name, name);
    strcpy(symbolTable[symbolCount].type, type);
    symbolCount++;
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
        if (tokens[i].type == TOKEN_KEYWORD &&
            (strcmp(tokens[i].value, "int") == 0 ||
             strcmp(tokens[i].value, "float") == 0 ||
             strcmp(tokens[i].value, "char") == 0 ||
             strcmp(tokens[i].value, "double") == 0 ||
             strcmp(tokens[i].value, "void") == 0))
        {
            if (tokens[i + 1].type == TOKEN_IDENTIFIER &&
                (i == 0 || strcmp(tokens[i - 1].value, "return") != 0))
            {
                declare(tokens[i + 1].value, tokens[i].value);
            }
        }
        // Suggest correction for possible misspelled keywords
        else if (tokens[i].type == TOKEN_KEYWORD) {
            const char *expectedKeywords[] = {"int", "float"};
            suggestKeyword(tokens[i].value, expectedKeywords, 2, tokens[i].line);
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

        // Type checking for assignments
        if (i >= 2 && tokens[i-1].type == TOKEN_ASSIGN && tokens[i-2].type == TOKEN_IDENTIFIER)
        {
            int varIndex = getVariableType(tokens[i-2].value);
            if (varIndex >= 0)
            {
                // Check for type compatibility
                if (strcmp(symbolTable[varIndex].type, "int") == 0)
                {
                    if (tokens[i].type == TOKEN_STRING)
                    {
                        printf("Semantic Error: Type mismatch - cannot assign string to int variable '%s' (line %d)\n",
                            tokens[i-2].value, tokens[i].line);
                        exit(1);
                    }
                }
            }
        }

        // Check for divide by zero
        if (i < count - 2 && tokens[i + 1].type == TOKEN_OPERATOR && strcmp(tokens[i + 1].value, "/") == 0) {
            if (tokens[i + 2].type == TOKEN_NUMBER && strcmp(tokens[i + 2].value, "0") == 0) {
                printf("Semantic Error: Division by zero at line %d\n", tokens[i].line);
                exit(1);
            } else if (tokens[i + 2].type == TOKEN_IDENTIFIER) {
                // For variables, we can only warn as we don't know the runtime value
                printf("Warning: Potential division by zero at line %d - check that '%s' is not zero\n", 
                       tokens[i].line, tokens[i + 2].value);
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

        // for loop header check
if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "for") == 0)
{
    if (tokens[i + 1].type != TOKEN_LPAREN)
    {
        printf("Semantic Error: Missing '(' after 'for'\n");
        exit(1);
    }

    int j = i + 2;
    int semicolonCount = 0;
    while (tokens[j].type != TOKEN_RPAREN && tokens[j].type != TOKEN_EOF)
    {
        if (tokens[j].type == TOKEN_IDENTIFIER && !isDeclared(tokens[j].value))
        {
            printf("Semantic Error: Undeclared variable '%s' in for loop\n", tokens[j].value);
            exit(1);
        }
        if (tokens[j].type == TOKEN_SEMICOLON)
            semicolonCount++;
        j++;
    }
    if (tokens[j].type != TOKEN_RPAREN)
    {
        printf("Semantic Error: Missing ')' in for loop\n");
        exit(1);
    }
    if (semicolonCount != 2)
    {
        printf("Semantic Error: Invalid for loop header (should contain two ';')\n");
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