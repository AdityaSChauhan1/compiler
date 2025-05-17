#include <stdio.h>
#include <string.h>
#include "lexer.h"

int labelCount = 0;
int tempVarCount = 0;

// Generate a new label
const char *newLabel(char *buffer)
{
    sprintf(buffer, "L%d", labelCount++);
    return buffer;
}

// Generate a new temporary variable
const char *newTempVar(char *buffer)
{
    sprintf(buffer, "t%d", tempVarCount++);
    return buffer;
}

void generateIR(Token tokens[], int count)
{
    FILE *out = fopen("ir.txt", "w");
    if (!out)
    {
        perror("ir.txt");
        return;
    }

    int i = 0;
    char labelBuffer[10];
    char tempVarBuffer[10];

    while (i < count)
    {
        // Skip declarations like: int a;
        if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "int") == 0 &&
            tokens[i + 1].type == TOKEN_IDENTIFIER &&
            tokens[i + 2].type == TOKEN_SEMICOLON)
        {
            i += 3;
        }

        // Handle assignments: a = b + c + d;
        else if (tokens[i].type == TOKEN_IDENTIFIER &&
                 tokens[i + 1].type == TOKEN_ASSIGN)
        {
            char lhs[64], op1[64], op2[64], operator[4];
            strcpy(lhs, tokens[i].value);
            i += 2; // Skip 'a' and '='

            // Handle complex expressions
            strcpy(op1, tokens[i].value);
            i++;

            while (tokens[i].type == TOKEN_OPERATOR)
            {
                strcpy(operator, tokens[i].value);
                i++;
                strcpy(op2, tokens[i].value);
                i++;

                // Generate a temporary variable for intermediate results
                const char *tempVar = newTempVar(tempVarBuffer);
                fprintf(out, "%s = %s %s %s\n", tempVar, op1, operator, op2);
                strcpy(op1, tempVar); // Use the result as the next operand
            }

            // Final assignment to the left-hand side
            fprintf(out, "%s = %s\n", lhs, op1);

            // Ensure we skip the semicolon
            if (tokens[i].type == TOKEN_SEMICOLON)
            {
                i++;
            }
        }

        // Handle if condition
        else if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "if") == 0)
        {
            char Ltrue[10], Lfalse[10];
            newLabel(Ltrue);
            newLabel(Lfalse);

            // if (condition)
            if (tokens[i + 1].type == TOKEN_LPAREN &&
                (tokens[i + 2].type == TOKEN_IDENTIFIER || tokens[i + 2].type == TOKEN_NUMBER) &&
                tokens[i + 3].type == TOKEN_OPERATOR &&
                (tokens[i + 4].type == TOKEN_IDENTIFIER || tokens[i + 4].type == TOKEN_NUMBER) &&
                tokens[i + 5].type == TOKEN_RPAREN)
            {
                fprintf(out, "if %s %s %s goto %s\n", tokens[i + 2].value, tokens[i + 3].value, tokens[i + 4].value, Ltrue);
                fprintf(out, "goto %s\n", Lfalse);
                fprintf(out, "%s:\n", Ltrue);
                i += 6;

                // Parse if block
                if (tokens[i].type == TOKEN_LBRACE)
                {
                    i++;
                    while (tokens[i].type != TOKEN_RBRACE && i < count)
                    {
                        // Recursively handle statements inside the block
                        if (tokens[i].type == TOKEN_IDENTIFIER &&
                            tokens[i + 1].type == TOKEN_ASSIGN &&
                            (tokens[i + 2].type == TOKEN_IDENTIFIER || tokens[i + 2].type == TOKEN_NUMBER))
                        {
                            char lhs[64], op1[64], op2[64], operator[4];
                            strcpy(lhs, tokens[i].value);
                            strcpy(op1, tokens[i + 2].value);

                            if (tokens[i + 3].type == TOKEN_OPERATOR &&
                                (tokens[i + 4].type == TOKEN_IDENTIFIER || tokens[i + 4].type == TOKEN_NUMBER) &&
                                tokens[i + 5].type == TOKEN_SEMICOLON)
                            {
                                strcpy(operator, tokens[i + 3].value);
                                strcpy(op2, tokens[i + 4].value);
                                fprintf(out, "%s = %s %s %s\n", lhs, op1, operator, op2);
                                i += 6;
                            }
                            else if (tokens[i + 3].type == TOKEN_SEMICOLON)
                            {
                                fprintf(out, "%s = %s\n", lhs, op1);
                                i += 4;
                            }
                            else
                            {
                                i++;
                            }
                        }
                        else
                        {
                            i++;
                        }
                    }
                    i++; // skip }
                }
                fprintf(out, "goto L2\n"); // Jump to the next block after the if-else
                fprintf(out, "%s:\n", Lfalse);
            }
        }

        // Handle while loop
        else if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "while") == 0)
        {
            char Lbegin[10], Lbody[10], Lend[10];
            newLabel(Lbegin);
            newLabel(Lbody);
            newLabel(Lend);

            fprintf(out, "%s:\n", Lbegin);

            // Parse while condition
            if (tokens[i + 1].type == TOKEN_LPAREN &&
                (tokens[i + 2].type == TOKEN_IDENTIFIER || tokens[i + 2].type == TOKEN_NUMBER) &&
                tokens[i + 3].type == TOKEN_OPERATOR &&
                (tokens[i + 4].type == TOKEN_IDENTIFIER || tokens[i + 4].type == TOKEN_NUMBER) &&
                tokens[i + 5].type == TOKEN_RPAREN)
            {
                fprintf(out, "if %s %s %s goto %s\n", tokens[i + 2].value, tokens[i + 3].value, tokens[i + 4].value, Lbody);
                fprintf(out, "goto %s\n", Lend);
                fprintf(out, "%s:\n", Lbody);
                i += 6;

                // Parse while block
                if (tokens[i].type == TOKEN_LBRACE)
                {
                    i++;
                    while (tokens[i].type != TOKEN_RBRACE && i < count)
                    {
                        // Recursively handle statements inside the block
                        if (tokens[i].type == TOKEN_IDENTIFIER &&
                            tokens[i + 1].type == TOKEN_ASSIGN &&
                            (tokens[i + 2].type == TOKEN_IDENTIFIER || tokens[i + 2].type == TOKEN_NUMBER))
                        {
                            char lhs[64], op1[64], op2[64], operator[4];
                            strcpy(lhs, tokens[i].value);
                            strcpy(op1, tokens[i + 2].value);

                            if (tokens[i + 3].type == TOKEN_OPERATOR &&
                                (tokens[i + 4].type == TOKEN_IDENTIFIER || tokens[i + 4].type == TOKEN_NUMBER) &&
                                tokens[i + 5].type == TOKEN_SEMICOLON)
                            {
                                strcpy(operator, tokens[i + 3].value);
                                strcpy(op2, tokens[i + 4].value);
                                fprintf(out, "%s = %s %s %s\n", lhs, op1, operator, op2);
                                i += 6;
                            }
                            else if (tokens[i + 3].type == TOKEN_SEMICOLON)
                            {
                                fprintf(out, "%s = %s\n", lhs, op1);
                                i += 4;
                            }
                            else
                            {
                                i++;
                            }
                        }
                        else
                        {
                            i++;
                        }
                    }
                    i++; // skip }
                }

                fprintf(out, "goto %s\n", Lbegin);
                fprintf(out, "%s:\n", Lend);
            }
        }

        // Handle return statement
        else if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "return") == 0 &&
                 (tokens[i + 1].type == TOKEN_IDENTIFIER || tokens[i + 1].type == TOKEN_NUMBER))
        {
            fprintf(out, "return %s\n", tokens[i + 1].value);
            i += 3;
        }

        else
        {
            i++;
        }
    }

    fclose(out);
    printf("Intermediate code written to ir.txt\n");
}