#ifndef TOKENS_H
#define TOKENS_H

#include <string>

// All token types - FINAL VERSION
enum TokenType {
    // --- System Tokens ---
    T_EOF = 0, T_ERROR,

    // --- Literals and Identifiers ---
    T_ID, T_INTLIT, T_FLOATLIT,
    // --- Keywords (including Type keywords) ---
    T_CLASS, T_IMPLEMENT, T_FUNC, T_CONSTRUCT,
    T_ATTRIBUTE, T_LOCAL, T_PUBLIC, T_PRIVATE,
    T_ISA, T_FLOAT_TYPE, T_INT_TYPE, T_VOID,
    T_SELF, T_THEN, T_ELSE, T_WHILE,
    T_IF, T_READ, T_WRITE, T_RETURN,
 
    // --- Operators ---
    T_OR, T_AND, T_NOT, T_EQ, T_NEQ, T_LT, T_GT, T_LE, T_GE,
    T_ASSIGN, T_ARROW, T_PLUS, T_MINUS, T_MUL, T_DIV,

    // --- Punctuation ---
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
    T_LBRACK, T_RBRACK, T_COLON, T_SEMI, T_COMMA, T_DOT
};

// Token structure
struct Token {
    TokenType type;
    std::string lexeme;
    int lineno;
};

#endif // TOKENS_H