#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "suggest.h" // Add this at the top

Token tokens[MAX_TOKENS];
int current = 0;

FILE *dotFile;
int nodeId = 0;

Token peek()
{
    return tokens[current];
}

int match(TokenType expected, const char *value)
{
    if (peek().type == expected && (!value || strcmp(peek().value, value) == 0))
    {
        current++;
        return 1;
    }
    return 0;
}

void syntaxError(const char *message)
{
    printf("Syntax Error: %s at '%s' (line %d)\n", message, peek().value, peek().line);
    exit(1);
}

int newNode(const char *label)
{
    int id = nodeId++;
    fprintf(dotFile, "  node%d [label=\"%s\"];\n", id, label);
    return id;
}

void addEdge(int from, int to)
{
    fprintf(dotFile, "  node%d -> node%d;\n", from, to);
}

int tokenNode(Token token)
{
    char label[128];
    snprintf(label, sizeof(label), "%s\\n[%s]", token.value, getTokenTypeName(token.type));
    return newNode(label);
}

// In parser.c, update the expr() function:
int expr()
{
    int node = newNode("expr");

    // First term
    if (peek().type == TOKEN_IDENTIFIER || peek().type == TOKEN_NUMBER)
    {
        addEdge(node, tokenNode(tokens[current]));
        current++;
    }
    else if (peek().type == TOKEN_LPAREN)
    {
        addEdge(node, tokenNode(tokens[current])); // Add '('
        current++;
        addEdge(node, expr()); // Parse inner expression
        if (!match(TOKEN_RPAREN, NULL))
            syntaxError("Expected ')'");
        addEdge(node, tokenNode(tokens[current - 1])); // Add ')'
    }
    else
    {
        syntaxError("Expected identifier, number, or '('");
    }

    // Handle operators and subsequent terms
    while (peek().type == TOKEN_OPERATOR)
    {
        addEdge(node, tokenNode(tokens[current])); // Add operator
        current++;
        addEdge(node, expr()); // Parse right side
    }

    return node;
}

int decl()
{
    int node = newNode("decl");
    if (!(match(TOKEN_KEYWORD, "int") ||
          match(TOKEN_KEYWORD, "float") ||
          match(TOKEN_KEYWORD, "char") ||
          match(TOKEN_KEYWORD, "double") ||
          match(TOKEN_KEYWORD, "void")))
    {
        // Suggest possible correction
        const char *expectedKeywords[] = {"int", "float", "char", "double", "void"};
        suggestKeyword(peek().value, expectedKeywords, 5, peek().line);
        syntaxError("Expected type specifier");
    }
    addEdge(node, tokenNode(tokens[current - 1]));

    if (!match(TOKEN_IDENTIFIER, NULL))
        syntaxError("Expected identifier");
    addEdge(node, tokenNode(tokens[current - 1]));

    if (!match(TOKEN_SEMICOLON, NULL)) // Ensure a semicolon is present
        syntaxError("Expected ';'");
    addEdge(node, tokenNode(tokens[current - 1]));

    return node;
}

int stmt()
{
    int node = newNode("stmt");

    if (match(TOKEN_IDENTIFIER, NULL)) // Handle assignment statements
    {
        addEdge(node, tokenNode(tokens[current - 1])); // Add the identifier node

        if (!match(TOKEN_ASSIGN, NULL)) // Match the '=' operator
            syntaxError("Expected '='");
        addEdge(node, tokenNode(tokens[current - 1])); // Add the '=' node

        addEdge(node, expr()); // Parse the expression after '='

        if (!match(TOKEN_SEMICOLON, NULL)) // Ensure the statement ends with a semicolon
            syntaxError("Expected ';'");
        addEdge(node, tokenNode(tokens[current - 1])); // Add the ';' node
    }
    else if (match(TOKEN_KEYWORD, "return")) // Handle return statements
    {
        addEdge(node, tokenNode(tokens[current - 1])); // Add the 'return' node

        addEdge(node, expr()); // Parse the return expression

        if (!match(TOKEN_SEMICOLON, NULL)) // Ensure the statement ends with a semicolon
            syntaxError("Expected ';'");
        addEdge(node, tokenNode(tokens[current - 1])); // Add the ';' node
    }
    else if (match(TOKEN_KEYWORD, "if")) // Handle if statements
    {
        addEdge(node, tokenNode(tokens[current - 1])); // Add the 'if' node

        if (!match(TOKEN_LPAREN, NULL)) // Match the '('
            syntaxError("Expected '(' after 'if'");
        addEdge(node, tokenNode(tokens[current - 1])); // Add the '(' node

        addEdge(node, expr()); // Parse the condition inside the parentheses

        if (!match(TOKEN_RPAREN, NULL)) // Match the ')'
            syntaxError("Expected ')' after condition");
        addEdge(node, tokenNode(tokens[current - 1])); // Add the ')' node

        addEdge(node, block()); // Parse the block after the 'if'

        if (match(TOKEN_KEYWORD, "else")) // Handle optional 'else' block
        {
            addEdge(node, tokenNode(tokens[current - 1])); // Add the 'else' node
            addEdge(node, block());                        // Parse the block after the 'else'
        }
    }
    else if (match(TOKEN_KEYWORD, "while")) // Handle while statements
    {
        addEdge(node, tokenNode(tokens[current - 1])); // Add the 'while' node

        if (!match(TOKEN_LPAREN, NULL)) // Match the '('
            syntaxError("Expected '(' after 'while'");
        addEdge(node, tokenNode(tokens[current - 1])); // Add the '(' node

        addEdge(node, expr()); // Parse the condition inside the parentheses

        if (!match(TOKEN_RPAREN, NULL)) // Match the ')'
            syntaxError("Expected ')' after condition");
        addEdge(node, tokenNode(tokens[current - 1])); // Add the ')' node

        addEdge(node, block()); // Parse the block after the 'while'
    }
    else if (match(TOKEN_KEYWORD, "for"))
    {
        addEdge(node, tokenNode(tokens[current - 1]));

        if (!match(TOKEN_LPAREN, NULL))
            syntaxError("Expected '(' after 'for'");
        addEdge(node, tokenNode(tokens[current - 1]));

        // Parse initialization (can be a declaration or assignment)
        if (peek().type == TOKEN_KEYWORD &&
            (strcmp(peek().value, "int") == 0 ||
             strcmp(peek().value, "float") == 0 ||
             strcmp(peek().value, "char") == 0 ||
             strcmp(peek().value, "double") == 0 ||
             strcmp(peek().value, "void") == 0))
        {
            addEdge(node, decl());
        }
        else if (peek().type == TOKEN_IDENTIFIER)
        {
            addEdge(node, stmt());
        }
        else if (!match(TOKEN_SEMICOLON, NULL))
        {
            syntaxError("Expected initialization or ';' in for loop");
        }

        // Parse condition
        if (peek().type != TOKEN_SEMICOLON)
            addEdge(node, expr());
        if (!match(TOKEN_SEMICOLON, NULL))
            syntaxError("Expected ';' after for loop condition");

        // Parse increment
        // if (peek().type != TOKEN_RPAREN)
        //     addEdge(node, expr());
        // if (!match(TOKEN_RPAREN, NULL))
        //     syntaxError("Expected ')' after for loop increment");
        // NEW CODE: handle assignment OR empty increment
        if (peek().type == TOKEN_IDENTIFIER)
        {
            // Parse assignment manually
            addEdge(node, tokenNode(tokens[current++])); // identifier

            if (!match(TOKEN_ASSIGN, NULL))
                syntaxError("Expected '=' in for loop increment");

            addEdge(node, tokenNode(tokens[current - 1])); // '='
            addEdge(node, expr());                         // Right-hand expression
        }
        else if (peek().type != TOKEN_RPAREN)
        {
            syntaxError("Expected assignment or ')' in for loop increment");
        }

        if (!match(TOKEN_RPAREN, NULL))
            syntaxError("Expected ')' after for loop increment");

        // addEdge(node, tokenNode(tokens[current - 1]));

        addEdge(node, tokenNode(tokens[current - 1]));
        addEdge(node, block());
    }
    else
    {
        syntaxError("Unknown statement");
    }
    return node;
}

int block()
{
    int node = newNode("block");

    if (!match(TOKEN_LBRACE, NULL))
        syntaxError("Expected '{'");
    addEdge(node, tokenNode(tokens[current - 1]));

    // Parse declarations inside the block
    while (peek().type == TOKEN_KEYWORD &&
           (strcmp(peek().value, "int") == 0 ||
            strcmp(peek().value, "float") == 0 ||
            strcmp(peek().value, "char") == 0 ||
            strcmp(peek().value, "double") == 0 ||
            strcmp(peek().value, "void") == 0))
    {
        addEdge(node, decl());
    }

    // Parse statements inside the block
    while (peek().type == TOKEN_IDENTIFIER || strcmp(peek().value, "return") == 0 ||
           strcmp(peek().value, "if") == 0 || strcmp(peek().value, "while") == 0 || strcmp(peek().value, "for") == 0)
    {
        addEdge(node, stmt());
    }

    if (!match(TOKEN_RBRACE, NULL))
        syntaxError("Expected '}'");
    addEdge(node, tokenNode(tokens[current - 1]));

    return node;
}

int func_def()
{
    int node = newNode("func_def");

    if (!(match(TOKEN_KEYWORD, "int") ||
          match(TOKEN_KEYWORD, "float") ||
          match(TOKEN_KEYWORD, "char") ||
          match(TOKEN_KEYWORD, "double") ||
          match(TOKEN_KEYWORD, "void")))
    {
        // Suggest possible correction
        const char *expectedKeywords[] = {"int", "float", "char", "double", "void"};
        suggestKeyword(peek().value, expectedKeywords, 5, peek().line);
        syntaxError("Expected type specifier");
    }
    addEdge(node, tokenNode(tokens[current - 1]));

    if (!match(TOKEN_IDENTIFIER, NULL))
        syntaxError("Expected function name");
    addEdge(node, tokenNode(tokens[current - 1]));

    if (!match(TOKEN_LPAREN, NULL))
        syntaxError("Expected '('");
    addEdge(node, tokenNode(tokens[current - 1]));

    // if (!match(TOKEN_RPAREN, NULL))
    //     syntaxError("Expected ')'");
    // Optional: handle parameter list (e.g., int x)
    if (peek().type == TOKEN_KEYWORD &&
        (strcmp(peek().value, "int") == 0 || strcmp(peek().value, "float") == 0 ||
         strcmp(peek().value, "char") == 0 || strcmp(peek().value, "double") == 0))
    {
        // type
        addEdge(node, tokenNode(tokens[current++]));

        // identifier
        if (!match(TOKEN_IDENTIFIER, NULL))
            syntaxError("Expected parameter name");

        addEdge(node, tokenNode(tokens[current - 1]));
    }

    // Match closing parenthesis
    if (!match(TOKEN_RPAREN, NULL))
        syntaxError("Expected ')'");
    //addEdge(node, tokenNode(tokens[current - 1]));

    addEdge(node, tokenNode(tokens[current - 1]));

    addEdge(node, block());

    return node;
}

int func_def_list()
{
    int node = newNode("func_def_list");
    while (peek().type == TOKEN_KEYWORD &&
           (strcmp(peek().value, "int") == 0 ||
            strcmp(peek().value, "float") == 0 ||
            strcmp(peek().value, "char") == 0 ||
            strcmp(peek().value, "double") == 0 ||
            strcmp(peek().value, "void") == 0))
    {
        addEdge(node, func_def());
    }
    return node;
}

int program()
{
    int node = newNode("program");
    addEdge(node, func_def_list());

    // if (peek().type != TOKEN_EOF)
    //     syntaxError("Expected EOF");

    return node;
}

void parseTokens(Token inputTokens[], int count)
{
    memcpy(tokens, inputTokens, sizeof(Token) * count);
    current = 0;
    nodeId = 0;

    dotFile = fopen("tree.dot", "w");
    fprintf(dotFile, "digraph ParseTree {\n");
    fprintf(dotFile, "  node [shape=box, fontname=\"Courier\"];\n");

    int root = program();

    fprintf(dotFile, "}\n");
    fclose(dotFile);

    printf("Parse tree generated: tree.dot\n");
}