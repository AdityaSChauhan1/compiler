#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct
{
    TokenType type;
    char value[64];
    int line; // Add line number
} Token;

#define MAX_TOKENS 1000

extern Token tokens[MAX_TOKENS];
extern int tokenCount;

void tokenize(const char *code);
const char *getTokenTypeName(TokenType type);
void writeTokensToFile(const char *filename);

#endif