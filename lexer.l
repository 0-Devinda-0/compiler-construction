%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

int yylineno = 1;   // track line numbers
Token currentToken;
%}

%option noyywrap

letters         [a-zA-Z]
digits          [0-9]    
nonzero_digits  [1-9]
alphanum        ({letters}|{digits}|_)
id              {letters}({alphanum})*
integer         (0|{nonzero_digits}{digits}*)
fraction        (\.{digits}+)
float           {integer}{fraction}?([eE][+-]?{integer})?

%%

"else"          { return T_ELSE; } // updated this part to return token instead of printing them in tma-1
"float"         { return T_FLOAT_TYPE; }
"func"          { return T_FUNC; }
"if"            { return T_IF; }
"implements"    { return T_IMPLEMENT; }
"class"         { return T_CLASS; }
"attribute"     { return T_ATTRIBUTE; }
"integer"       { return T_INT_TYPE; }
"isa"           { return T_ISA; }
"private"       { return T_PRIVATE; }
"public"        { return T_PUBLIC; }
"read"          { return T_READ; }
"return"        { return T_RETURN; }
"self"          { return T_SELF; }
"construct"     { return T_CONSTRUCT; }
"then"          { return T_THEN; }
"local"         { return T_LOCAL; }
"void"          { return T_VOID; }
"while"         { return T_WHILE; }
"write"         { return T_WRITE; }

"+"           { return T_PLUS; }
"-"           { return T_MINUS; }
"*"           { return T_MUL; }
"/"           { return T_DIV; }
":="          { return T_ASSIGN; }
"=="          { return T_EQ; }
"<>"          { return T_NEQ; }
"<"           { return T_LT; }
">"           { return T_GT; }
"<="          { return T_LE; }
">="          { return T_GE; }
"or"          { return T_OR; }
"and"         { return T_AND; }
"not"         { return T_NOT; }

"("           { return T_LPAREN; }
")"           { return T_RPAREN; }
"{"           { return T_LBRACE; }
"}"           { return T_RBRACE; }
"["           { return T_LBRACK; }
"]"           { return T_RBRACK; }
";"           { return T_SEMI; }
","           { return T_COMMA; }
"."           { return T_DOT; }
":"           { return T_COLON; }
"=>"          { return T_ARROW; }

"//".* ;

{integer}     { return T_INTLIT; }
{float}       { return T_FLOATLIT; }
{id}          { return T_ID; }



[ \t\r]+      ;   // ignore whitespace
\n            { yylineno++; }
.             { fprintf(stderr, "Unknown char: %s at line %d\n", yytext, yylineno); return T_ERROR; }

%%

// Wrapper for parser
Token getNextToken(){
    Token t;
    t.type = (TokenType)yylex();
    t.lexeme = yytext;
    t.lineno = yylineno;
    return t;
}
