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

// Helper function to get operator precedence
int precedence(const char *op)
{
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0)
        return 2;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0)
        return 1;
    return 0;
}

// Process an expression with proper operator precedence
void processExpression(FILE *out, Token tokens[], int *i, int count, const char *lhs, char *resultVar)
{
    char opStack[100][10];
    char valStack[100][64];
    int opTop = -1, valTop = -1;
    char tempVarBuffer[10];

    while (*i < count && tokens[*i].type != TOKEN_SEMICOLON &&
           tokens[*i].type != TOKEN_RPAREN && tokens[*i].type != TOKEN_RBRACE)
    {
        // Handle values (identifiers or numbers)
        if (tokens[*i].type == TOKEN_IDENTIFIER || tokens[*i].type == TOKEN_NUMBER)
        {
            valTop++;
            strcpy(valStack[valTop], tokens[*i].value);
            (*i)++;
        }
        // Handle operators
        else if (tokens[*i].type == TOKEN_OPERATOR)
        {
            while (opTop >= 0 && precedence(tokens[*i].value) <= precedence(opStack[opTop]))
            {
                // Pop operator and operands
                char op[10], op1[64], op2[64], tempVar[10];
                strcpy(op, opStack[opTop--]);
                strcpy(op2, valStack[valTop--]);
                strcpy(op1, valStack[valTop--]);

                // Generate temporary variable
                const char *temp = newTempVar(tempVar);
                fprintf(out, "%s = %s %s %s\n", temp, op1, op, op2);

                // Push result back
                valTop++;
                strcpy(valStack[valTop], temp);
            }
            opTop++;
            strcpy(opStack[opTop], tokens[*i].value);
            (*i)++;
        }
        // Handle parentheses
        else if (tokens[*i].type == TOKEN_LPAREN)
        {
            opTop++;
            strcpy(opStack[opTop], "(");
            (*i)++;
        }
        else if (tokens[*i].type == TOKEN_RPAREN)
        {
            while (opTop >= 0 && strcmp(opStack[opTop], "(") != 0)
            {
                char op[10], op1[64], op2[64], tempVar[10];
                strcpy(op, opStack[opTop--]);
                strcpy(op2, valStack[valTop--]);
                strcpy(op1, valStack[valTop--]);

                const char *temp = newTempVar(tempVar);
                fprintf(out, "%s = %s %s %s\n", temp, op1, op, op2);

                valTop++;
                strcpy(valStack[valTop], temp);
            }
            if (opTop >= 0 && strcmp(opStack[opTop], "(") == 0)
                opTop--; // Pop the '('
            (*i)++;
        }
        else
        {
            break;
        }
    }

    // Process remaining operators
    while (opTop >= 0)
    {
        char op[10], op1[64], op2[64], tempVar[10];
        strcpy(op, opStack[opTop--]);
        strcpy(op2, valStack[valTop--]);
        strcpy(op1, valStack[valTop--]);

        const char *temp = newTempVar(tempVar);
        fprintf(out, "%s = %s %s %s\n", temp, op1, op, op2);

        valTop++;
        strcpy(valStack[valTop], temp);
    }

    // Final assignment
    if (valTop == 0)
    {
        if (lhs != NULL)
        {
            fprintf(out, "%s = %s\n", lhs, valStack[valTop]);
        }
        if (resultVar != NULL)
        {
            strcpy(resultVar, valStack[valTop]);
        }
    }
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
    static int returnEmitted = 0;

    while (i < count)
    {
        // Skip declarations like: int a;
        if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "int") == 0 &&
            tokens[i + 1].type == TOKEN_IDENTIFIER &&
            tokens[i + 2].type == TOKEN_SEMICOLON)
        {
            i += 3;
        }

        // Handle assignments: a = b + c * d;
        else if (tokens[i].type == TOKEN_IDENTIFIER &&
                 tokens[i + 1].type == TOKEN_ASSIGN)
        {
            char lhs[64];
            strcpy(lhs, tokens[i].value);
            i += 2; // Skip 'a' and '='
            char condResult[64];
            processExpression(out, tokens, &i, count, lhs, condResult);

            // Skip semicolon if present
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
            if (tokens[i + 1].type == TOKEN_LPAREN)
            {
                i += 2; // Skip 'if' and '('

                char condResult[64];
                processExpression(out, tokens, &i, count, NULL, condResult);

                if (tokens[i].type == TOKEN_RPAREN)
                {
                    // fprintf(out, "if %s goto %s\n", valStack[valTop], Ltrue);
                    fprintf(out, "if %s goto %s\n", condResult, Ltrue);
                    fprintf(out, "goto %s\n", Lfalse);
                    fprintf(out, "%s:\n", Ltrue);
                    i++; // Skip ')'
                }
                else
                {
                    printf("Error: Missing ')' in if condition\n");
                    break;
                }

                // Parse if block
                if (tokens[i].type == TOKEN_LBRACE)
                {
                    i++;
                    while (tokens[i].type != TOKEN_RBRACE && i < count)
                    {
                        // Recursively handle statements inside the block
                        if (tokens[i].type == TOKEN_IDENTIFIER &&
                            tokens[i + 1].type == TOKEN_ASSIGN)
                        {
                            char lhs[64];
                            strcpy(lhs, tokens[i].value);
                            i += 2; // Skip identifier and '='
                            processExpression(out, tokens, &i, count, lhs, condResult);
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
            if (tokens[i + 1].type == TOKEN_LPAREN)
            {
                i += 2; // Skip 'while' and '('

                char condResult[64];
                processExpression(out, tokens, &i, count, NULL, condResult);

                if (tokens[i].type == TOKEN_RPAREN)
                {
                    // fprintf(out, "if %s goto %s\n", valStack[valTop], Lbody);
                    fprintf(out, "if %s goto %s\n", condResult, Lbody);
                    fprintf(out, "goto %s\n", Lend);
                    fprintf(out, "%s:\n", Lbody);
                    i++; // Skip ')'
                }
                else
                {
                    printf("Error: Missing ')' in while condition\n");
                    break;
                }

                // Parse while block
                if (tokens[i].type == TOKEN_LBRACE)
                {
                    i++;
                    while (tokens[i].type != TOKEN_RBRACE && i < count)
                    {
                        // Recursively handle statements inside the block
                        if (tokens[i].type == TOKEN_IDENTIFIER &&
                            tokens[i + 1].type == TOKEN_ASSIGN)
                        {
                            char lhs[64];
                            strcpy(lhs, tokens[i].value);
                            i += 2; // Skip identifier and '='
                            processExpression(out, tokens, &i, count, lhs, condResult);
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
        else if (tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "for") == 0)
        {
            char Lbegin[10], Lcond[10], Lbody[10], Lend[10];
            newLabel(Lbegin);
            newLabel(Lcond);
            newLabel(Lbody);
            newLabel(Lend);

            // Match '('
            if (tokens[i + 1].type == TOKEN_LPAREN)
            {
                i += 2; // skip 'for' and '('

                // 1. Initialization (a = 0;)
                if (tokens[i].type == TOKEN_IDENTIFIER &&
                    tokens[i + 1].type == TOKEN_ASSIGN)
                {
                    char lhs[64];
                    strcpy(lhs, tokens[i].value);
                    i += 2; // skip identifier and '='
                    processExpression(out, tokens, &i, count, lhs, NULL);
                    if (tokens[i].type == TOKEN_SEMICOLON)
                        i++;
                }

                fprintf(out, "%s:\n", Lcond);

                // 2. Condition (i < 10;)
                char condResult[64] = "1"; // Default to true if no condition
                if (tokens[i].type != TOKEN_SEMICOLON)
                {
                    processExpression(out, tokens, &i, count, NULL, condResult);
                }

                if (tokens[i].type == TOKEN_SEMICOLON)
                    i++;

                fprintf(out, "if %s goto %s\n", condResult, Lbody);
                fprintf(out, "goto %s\n", Lend);

                fprintf(out, "%s:\n", Lbody);

                // Save increment expression start
                int incrementStart = i;

                // Skip increment expression for now
                while (tokens[i].type != TOKEN_RPAREN && i < count)
                    i++;
                i++; // skip ')'

                // 3. Loop body
                if (tokens[i].type == TOKEN_LBRACE)
                {
                    i++;
                    while (tokens[i].type != TOKEN_RBRACE && i < count)
                    {
                        if (tokens[i].type == TOKEN_IDENTIFIER &&
                            tokens[i + 1].type == TOKEN_ASSIGN)
                        {
                            char lhs[64];
                            strcpy(lhs, tokens[i].value);
                            i += 2;
                            processExpression(out, tokens, &i, count, lhs, NULL);
                            if (tokens[i].type == TOKEN_SEMICOLON)
                                i++;
                        }
                        else
                        {
                            i++;
                        }
                    }
                    i++; // skip '}'
                }

                // 4. Emit increment expression
                fprintf(out, "%s:\n", Lbegin);
                int tempI = incrementStart;
                while (tokens[tempI].type != TOKEN_RPAREN && tempI < count)
                {
                    if (tokens[tempI].type == TOKEN_IDENTIFIER &&
                        tokens[tempI + 1].type == TOKEN_ASSIGN)
                    {
                        char lhs[64];
                        strcpy(lhs, tokens[tempI].value);
                        tempI += 2;
                        processExpression(out, tokens, &tempI, count, lhs, NULL);
                        if (tokens[tempI].type == TOKEN_SEMICOLON)
                            tempI++;
                    }
                    else
                    {
                        tempI++;
                    }
                }

                fprintf(out, "goto %s\n", Lcond);
                fprintf(out, "%s:\n", Lend);
            }
        }

        // Handle return statement
        else if (!returnEmitted && tokens[i].type == TOKEN_KEYWORD && strcmp(tokens[i].value, "return") == 0 &&
                 (tokens[i + 1].type == TOKEN_IDENTIFIER || tokens[i + 1].type == TOKEN_NUMBER))
        {
            fprintf(out, "return %s\n", tokens[i + 1].value);
            returnEmitted = 1;
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