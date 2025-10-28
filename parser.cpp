
#include <bits/stdc++.h>
#include "tokens.h"
using namespace std;

// global derivation file
static FILE* DERIV = nullptr;

// current token (lookahead.type)
static Token lookahead;
Token getNextToken();

// helper: token name for readable messages
const char* tokenName(int t) {
  switch(t){
    case T_EOF: return "$";
    case T_CLASS: return "class"; case T_IMPLEMENT: return "implement";
    case T_FUNC: return "func"; case T_CONSTRUCT: return "constructor";
    case T_ATTRIBUTE: return "attribute"; case T_LOCAL: return "local";
    case T_PUBLIC: return "public"; case T_PRIVATE: return "private";
    case T_ISA: return "isa"; case T_INT_TYPE: return "integer";
    case T_FLOAT_TYPE: return "float"; case T_VOID: return "void";
    case T_SELF: return "self"; case T_THEN: return "then";
    case T_ELSE: return "else"; case T_WHILE: return "while";
    case T_IF: return "if"; case T_READ: return "read"; case T_WRITE: return "write";
    case T_RETURN: return "return"; case T_OR: return "or"; case T_AND: return "and";
    case T_NOT: return "not"; case T_EQ: return "=="; case T_NEQ: return "<>";
    case T_LT: return "<"; case T_GT: return ">"; case T_LE: return "<="; case T_GE: return ">=";
    case T_ASSIGN: return ":="; case T_ARROW: return "=>"; case T_PLUS: return "+";
    case T_MINUS: return "-"; case T_MUL: return "*"; case T_DIV: return "/";
    case T_LBRACE: return "{"; case T_RBRACE: return "}"; case T_LPAREN: return "(";
    case T_RPAREN: return ")"; case T_LBRACK: return "["; case T_RBRACK: return "]";
    case T_COLON: return ":"; case T_SEMI: return ";"; case T_COMMA: return ",";
    case T_DOT: return "."; case T_ID: return "id"; case T_INTLIT: return "intLit";
    case T_FLOATLIT: return "floatLit"; case T_ERROR: return "ERROR";
    default: return "?";
  }
}

// forward declarations (one function per nonterminal)
void prog();
void classOrImplOrFuncList();
void classOrImplOrFunc();
void classDecl(); void classDeclOptIsa(); void idListTailOpt(); void classMembers(); void classMember(); void visibility();
void memberDecl(); void funcDecl(); void funcHead(); void returnType();
void implDef(); void funcDefList(); void funcDef(); void funcBody();
void localVarDeclOrStmtList(); void localVarDeclOrStmt(); void localVarDecl();
void attributeDecl(); void varDecl(); void arraySizeList(); void arraySize(); void type();
void statement(); void idOrSelfPrefix(); void idnestList(); void idnestElem(); void idOrSelf();
void stmtAfterIdOrSelf(); void assignOp(); void statBlock(); void statementList();
void expr(); void relExpr(); void relExprPrime(); void relOp();
void arithExpr(); void arithExprPrime(); void addOp();
void term(); void termPrime(); void multOp();
void factor(); void sign(); void factorAfterIdOrSelf();
void variable(); void variableTail(); void indice(); void indiceList();
void functionCall(); void aParams(); void aParamsTailOpt(); void fParams(); void param(); void fParamsTailOpt();


// helpers
static void advance(){ lookahead = getNextToken(); }
static void expect(int t){
  if(lookahead.type != t){
    fprintf(stderr, "Syntax error: expected %s, got %s (lexeme=\"%s\") at line %d\n",
            tokenName(t), tokenName(lookahead.type), lookahead.lexeme.c_str(), lookahead.lineno);
    exit(1);
  }
  advance();
}
static void note(const string& s){ if(DERIV) fprintf(DERIV, "%s\n", s.c_str()); }

// --- parser implementations ---
void prog(){ note("prog -> classOrImplOrFuncList"); classOrImplOrFuncList(); }

void classOrImplOrFuncList(){
  if(lookahead.type==T_CLASS||lookahead.type==T_IMPLEMENT||lookahead.type==T_FUNC||lookahead.type==T_CONSTRUCT){
    note("classOrImplOrFuncList -> classOrImplOrFunc classOrImplOrFuncList");
    classOrImplOrFunc(); classOrImplOrFuncList();
  } else note("classOrImplOrFuncList -> epsilon");
}

void classOrImplOrFunc(){
  if(lookahead.type==T_CLASS){ note("classOrImplOrFunc -> classDecl"); classDecl(); }
  else if(lookahead.type==T_IMPLEMENT){ note("classOrImplOrFunc -> implDef"); implDef(); }
  else { note("classOrImplOrFunc -> funcDef"); funcDef(); }
}

/* classDecl */
void classDecl(){
  note("classDecl -> class id classDeclOptIsa { classMembers } ;");
  expect(T_CLASS); expect(T_ID);
  classDeclOptIsa();
  expect(T_LBRACE); classMembers(); expect(T_RBRACE); expect(T_SEMI);
}
void classDeclOptIsa(){
  if(lookahead.type==T_ISA){ note("classDeclOptIsa -> isa id idListTailOpt"); expect(T_ISA); expect(T_ID); idListTailOpt(); }
  else note("classDeclOptIsa -> epsilon");
}
void idListTailOpt(){
  if(lookahead.type==T_COMMA){ note("idListTailOpt -> , id idListTailOpt"); expect(T_COMMA); expect(T_ID); idListTailOpt(); }
  else note("idListTailOpt -> epsilon");
}
void classMembers(){
  if(lookahead.type==T_PUBLIC||lookahead.type==T_PRIVATE){ note("classMembers -> classMember classMembers"); classMember(); classMembers(); }
  else note("classMembers -> epsilon");
}
void classMember(){ note("classMember -> visibility memberDecl"); visibility(); memberDecl(); }
void visibility(){ if(lookahead.type==T_PUBLIC){ note("visibility -> public"); expect(T_PUBLIC);} else { note("visibility -> private"); expect(T_PRIVATE);} }
void memberDecl(){ if(lookahead.type==T_ATTRIBUTE){ note("memberDecl -> attributeDecl"); attributeDecl(); } else { note("memberDecl -> funcDecl"); funcDecl(); } }
void funcDecl(){ note("funcDecl -> funcHead ;"); funcHead(); expect(T_SEMI); }
void funcHead(){
  if(lookahead.type==T_FUNC){
    note("funcHead -> func id ( fParams ) => returnType");
    expect(T_FUNC); expect(T_ID); expect(T_LPAREN); fParams(); expect(T_RPAREN); expect(T_ARROW); returnType();
  } else {
    note("funcHead -> constructor ( fParams )");
    expect(T_CONSTRUCT); expect(T_LPAREN); fParams(); expect(T_RPAREN);
  }
}
void returnType(){ if(lookahead.type==T_VOID){ note("returnType -> void"); expect(T_VOID);} else { note("returnType -> type"); type(); } }

/* implDef */
void implDef(){ note("implDef -> implement id { funcDefList }"); expect(T_IMPLEMENT); expect(T_ID); expect(T_LBRACE); funcDefList(); expect(T_RBRACE); }
void funcDefList(){ if(lookahead.type==T_FUNC||lookahead.type==T_CONSTRUCT){ note("funcDefList -> funcDef funcDefList"); funcDef(); funcDefList(); } else note("funcDefList -> epsilon"); }
void funcDef(){ note("funcDef -> funcHead funcBody"); funcHead(); funcBody(); }
void funcBody(){ note("funcBody -> { localVarDeclOrStmtList }"); expect(T_LBRACE); localVarDeclOrStmtList(); expect(T_RBRACE); }
void localVarDeclOrStmtList(){ if(lookahead.type==T_LOCAL||lookahead.type==T_IF||lookahead.type==T_WHILE||lookahead.type==T_READ||lookahead.type==T_WRITE||lookahead.type==T_RETURN||lookahead.type==T_ID||lookahead.type==T_SELF||lookahead.type==T_LBRACE){ note("localVarDeclOrStmtList -> localVarDeclOrStmt localVarDeclOrStmtList"); localVarDeclOrStmt(); localVarDeclOrStmtList(); } else note("localVarDeclOrStmtList -> epsilon"); }
void localVarDeclOrStmt(){ if(lookahead.type==T_LOCAL){ note("localVarDeclOrStmt -> localVarDecl"); localVarDecl(); } else { note("localVarDeclOrStmt -> statement"); statement(); } }
void localVarDecl(){ note("localVarDecl -> local varDecl"); expect(T_LOCAL); varDecl(); }
void attributeDecl(){ note("attributeDecl -> attribute varDecl"); expect(T_ATTRIBUTE); varDecl(); }
void varDecl(){ note("varDecl -> id : type arraySizeList ;"); expect(T_ID); expect(T_COLON); type(); arraySizeList(); expect(T_SEMI); }
void arraySizeList(){ if(lookahead.type==T_LBRACK){ note("arraySizeList -> arraySize arraySizeList"); arraySize(); arraySizeList(); } else note("arraySizeList -> epsilon"); }
void arraySize(){ note("arraySize -> [ intLit ] | [ ]"); expect(T_LBRACK); if(lookahead.type==T_INTLIT){ expect(T_INTLIT);} expect(T_RBRACK); }
void type(){ if(lookahead.type==T_INT_TYPE){ note("type -> integer"); expect(T_INT_TYPE);} else if(lookahead.type==T_FLOAT_TYPE){ note("type -> float"); expect(T_FLOAT_TYPE);} else { note("type -> id"); expect(T_ID);} }

/* statements and blocks */
void statement(){
  if(lookahead.type==T_IF){
    note("statement -> if ( relExpr ) then statBlock else statBlock ;");
    expect(T_IF); expect(T_LPAREN); relExpr(); expect(T_RPAREN);
    expect(T_THEN); statBlock(); expect(T_ELSE); statBlock(); expect(T_SEMI);
  } else if(lookahead.type==T_WHILE){
    note("statement -> while ( relExpr ) statBlock ;");
    expect(T_WHILE); expect(T_LPAREN); relExpr(); expect(T_RPAREN); statBlock(); expect(T_SEMI);
  } else if(lookahead.type==T_READ){
    note("statement -> read ( variable ) ;");
    expect(T_READ); expect(T_LPAREN); variable(); expect(T_RPAREN); expect(T_SEMI);
  } else if(lookahead.type==T_WRITE){
    note("statement -> write ( expr ) ;");
    expect(T_WRITE); expect(T_LPAREN); expr(); expect(T_RPAREN); expect(T_SEMI);
  } else if(lookahead.type==T_RETURN){
    note("statement -> return ( expr ) ;");
    expect(T_RETURN); expect(T_LPAREN); expr(); expect(T_RPAREN); expect(T_SEMI);
  } else if(lookahead.type==T_ID||lookahead.type==T_SELF){
    note("statement -> idOrSelfPrefix stmtAfterIdOrSelf");
    idOrSelfPrefix(); stmtAfterIdOrSelf();
  } else if(lookahead.type==T_LBRACE || lookahead.type==T_RBRACE){
    note("statement -> epsilon");
    // epsilon; (used for empty statBlock option)
  } else {
    fprintf(stderr, "Unexpected token in statement: %s\n", tokenName(lookahead.type)); exit(1);
  }
}

void idOrSelfPrefix(){
  note("idOrSelfPrefix -> idnestList id");
  idnestList(); expect(T_ID);
}
void idnestList(){ if(lookahead.type==T_ID||lookahead.type==T_SELF){ note("idnestList -> idnestElem idnestList"); idnestElem(); idnestList(); } else note("idnestList -> epsilon"); }
void idnestElem(){
  // idOrSelf indiceList .  |  idOrSelf ( aParams ) .
  note("idnestElem -> idOrSelf indiceList . | idOrSelf ( aParams ) .");
  idOrSelf();
  if(lookahead.type==T_LPAREN){ expect(T_LPAREN); aParams(); expect(T_RPAREN); expect(T_DOT); }
  else { indiceList(); expect(T_DOT); }
}
void idOrSelf(){ if(lookahead.type==T_ID){ note("idOrSelf -> id"); expect(T_ID);} else { note("idOrSelf -> self"); expect(T_SELF);} }

void stmtAfterIdOrSelf(){
  if(lookahead.type==T_LPAREN){ note("stmtAfterIdOrSelf -> ( aParams ) ;"); expect(T_LPAREN); aParams(); expect(T_RPAREN); expect(T_SEMI); }
  else { note("stmtAfterIdOrSelf -> variableTail assignOp expr ;"); variableTail(); assignOp(); expr(); expect(T_SEMI); }
}
void assignOp(){ note("assignOp -> :="); expect(T_ASSIGN); }

void statBlock(){
  if(lookahead.type==T_LBRACE){ note("statBlock -> { statementList }"); expect(T_LBRACE); statementList(); expect(T_RBRACE); }
  else if(lookahead.type==T_IF||lookahead.type==T_WHILE||lookahead.type==T_READ||lookahead.type==T_WRITE||lookahead.type==T_RETURN||lookahead.type==T_ID||lookahead.type==T_SELF){ note("statBlock -> statement"); statement(); }
  else { note("statBlock -> epsilon"); }
}
void statementList(){ if(lookahead.type==T_IF||lookahead.type==T_WHILE||lookahead.type==T_READ||lookahead.type==T_WRITE||lookahead.type==T_RETURN||lookahead.type==T_ID||lookahead.type==T_SELF||lookahead.type==T_LBRACE){ note("statementList -> statement statementList"); statement(); statementList(); } else note("statementList -> epsilon"); }

/* expressions: relExpr -> arithExpr [relOp arithExpr] */
void expr(){ note("expr -> relExpr"); relExpr(); }
void relExpr(){ note("relExpr -> arithExpr relExpr'"); arithExpr(); relExprPrime(); }
void relExprPrime(){ if(lookahead.type==T_EQ||lookahead.type==T_NEQ||lookahead.type==T_LT||lookahead.type==T_GT||lookahead.type==T_LE||lookahead.type==T_GE){ note("relExpr' -> relOp arithExpr"); relOp(); arithExpr(); } else note("relExpr' -> epsilon"); }
void relOp(){ if(lookahead.type==T_EQ||lookahead.type==T_NEQ||lookahead.type==T_LT||lookahead.type==T_GT||lookahead.type==T_LE||lookahead.type==T_GE){ note("relOp"); advance(); } else { fprintf(stderr,"relOp expected\n"); exit(1);} }

/* arithExpr -> term arithExpr' ; arithExpr' -> addOp term arithExpr' | epsilon */
void arithExpr(){ note("arithExpr -> term arithExpr'"); term(); arithExprPrime(); }
void arithExprPrime(){ if(lookahead.type==T_PLUS||lookahead.type==T_MINUS||lookahead.type==T_OR){ note("arithExpr' -> addOp term arithExpr'"); addOp(); term(); arithExprPrime(); } else note("arithExpr' -> epsilon"); }
void addOp(){ if(lookahead.type==T_PLUS||lookahead.type==T_MINUS||lookahead.type==T_OR){ note("addOp"); advance(); } else { fprintf(stderr,"addOp expected\n"); exit(1);} }

/* term -> factor term' ; term' -> multOp factor term' | epsilon */
void term(){ note("term -> factor term'"); factor(); termPrime(); }
void termPrime(){ if(lookahead.type==T_MUL||lookahead.type==T_DIV||lookahead.type==T_AND){ note("term' -> multOp factor term'"); multOp(); factor(); termPrime(); } else note("term' -> epsilon"); }
void multOp(){ if(lookahead.type==T_MUL||lookahead.type==T_DIV||lookahead.type==T_AND){ note("multOp"); advance(); } else { fprintf(stderr,"multOp expected\n"); exit(1);} }

/* factor */
void factor(){
  if(lookahead.type==T_INTLIT){ note("factor -> intLit"); expect(T_INTLIT); }
  else if(lookahead.type==T_FLOATLIT){ note("factor -> floatLit"); expect(T_FLOATLIT); }
  else if(lookahead.type==T_LPAREN){ note("factor -> ( arithExpr )"); expect(T_LPAREN); arithExpr(); expect(T_RPAREN); }
  else if(lookahead.type==T_NOT){ note("factor -> not factor"); expect(T_NOT); factor(); }
  else if(lookahead.type==T_PLUS||lookahead.type==T_MINUS){ note("factor -> sign factor"); sign(); factor(); }
  else if(lookahead.type==T_ID||lookahead.type==T_SELF){ note("factor -> idOrSelfPrefix factorAfterIdOrSelf"); idOrSelfPrefix(); factorAfterIdOrSelf(); }
  else { fprintf(stderr,"factor: unexpected %s\n", tokenName(lookahead.type)); exit(1); }
}
void sign(){ if(lookahead.type==T_PLUS||lookahead.type==T_MINUS){ advance(); } else { fprintf(stderr,"sign expected\n"); exit(1);} }
void factorAfterIdOrSelf(){ if(lookahead.type==T_LPAREN){ note("factorAfterIdOrSelf -> ( aParams )"); expect(T_LPAREN); aParams(); expect(T_RPAREN); } else { note("factorAfterIdOrSelf -> variableTail"); variableTail(); } }

/* variable and indices */
void variable(){ note("variable -> idOrSelfPrefix variableTail"); idOrSelfPrefix(); variableTail(); }
void variableTail(){ note("variableTail -> indiceList"); indiceList(); }
void indice(){ note("indice -> [ arithExpr ]"); expect(T_LBRACK); arithExpr(); expect(T_RBRACK); }
void indiceList(){ if(lookahead.type==T_LBRACK){ note("indiceList -> indice indiceList"); indice(); indiceList(); } else note("indiceList -> epsilon"); }

/* functionCall */
void functionCall(){ note("functionCall -> idOrSelfPrefix ( aParams )"); idOrSelfPrefix(); expect(T_LPAREN); aParams(); expect(T_RPAREN); }

/* aParams and fParams */
void aParams(){ if(lookahead.type==T_LPAREN||lookahead.type==T_NOT||lookahead.type==T_PLUS||lookahead.type==T_MINUS||lookahead.type==T_INTLIT||lookahead.type==T_FLOATLIT||lookahead.type==T_ID||lookahead.type==T_SELF){ note("aParams -> expr aParamsTailOpt"); expr(); aParamsTailOpt(); } else note("aParams -> epsilon"); }
void aParamsTailOpt(){ if(lookahead.type==T_COMMA){ note("aParamsTailOpt -> , expr aParamsTailOpt"); expect(T_COMMA); expr(); aParamsTailOpt(); } else note("aParamsTailOpt -> epsilon"); }
void fParams(){ if(lookahead.type==T_ID){ note("fParams -> param fParamsTailOpt"); param(); fParamsTailOpt(); } else note("fParams -> epsilon"); }
void param(){ note("param -> id : type arraySizeList"); expect(T_ID); expect(T_COLON); type(); arraySizeList(); }
void fParamsTailOpt(){ if(lookahead.type==T_COMMA){ note("fParamsTailOpt -> , param fParamsTailOpt"); expect(T_COMMA); param(); fParamsTailOpt(); } else note("fParamsTailOpt -> epsilon"); }

// --- main ---
int main(int argc, char** argv){
  if(argc<2){ fprintf(stderr, "usage: %s <source-file>\n", argv[0]); return 1; }
  // open source file
  FILE* src = fopen(argv[1], "r");
  if(!src){ perror("fopen"); return 1; }
  // let flex know to read from this file
  extern FILE* yyin;
  yyin = src;

  DERIV = fopen("derivation.txt","w");
  if(!DERIV){ perror("deriv open"); return 1; }

  advance();             // load first token
  note("Start Derivation:");
  prog();                // parse
  if(lookahead.type != T_EOF){
    fprintf(stderr,"Extra input after program: %s\n", lookahead.lexeme.c_str());
    return 1;
}
  fclose(DERIV);
  fclose(src);
  printf("OK: syntax correct. Derivation written to derivation.txt\n");
  return 0;
}
