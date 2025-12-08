#include <bits/stdc++.h>
#include "tokens.h"
#include "ast.h"
#include "SemanticAnalyzer.h"
#include "codeGen.h"
using namespace std;

// global derivation file
static FILE* DERIV = nullptr;

// current token (lookahead)
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

// --- NEW FORWARD DECLARATIONS ---
AstNode* prog();
std::vector<AstNode*> classOrImplOrFuncList();
AstNode* classOrImplOrFunc();
AstNode* classDecl(); 
std::vector<IdentifierNode*> classDeclOptIsa(); 
std::vector<IdentifierNode*> idListTailOpt(); 
std::vector<AstNode*> classMembers(); 
AstNode* classMember(); 
std::string visibility();
AstNode* memberDecl(); 
AstNode* funcDecl(); 
AstNode* funcHead(); 
std::string returnType();
AstNode* implDef(); 
std::vector<AstNode*> funcDefList(); 
AstNode* funcDef(); 
BlockNode* funcBody();
std::vector<AstNode*> localVarDeclOrStmtList(); 
AstNode* localVarDeclOrStmt(); 
AstNode* localVarDecl();
AstNode* attributeDecl(); 
AstNode* varDecl(); 
bool arraySizeList(); 
bool arraySize(); 
std::string type();
AstNode* statement(); 
AstNode* parseIdOrSelfStatement();
AstNode* assignOp(AstNode* var); 
BlockNode* statBlock(); 
std::vector<AstNode*> statementList();
AstNode* expr(); 
AstNode* relExpr(); 
AstNode* relExprPrime(AstNode* leftNode); 
std::string relOp();
AstNode* arithExpr(); 
AstNode* arithExprPrime(AstNode* leftNode); 
std::string addOp();
AstNode* term(); 
AstNode* termPrime(AstNode* leftNode); 
std::string multOp();
AstNode* factor(); 
AstNode* sign(); 
AstNode* parseVariableAccess(AstNode* base);
AstNode* variable(); 
AstNode* indice(); 
std::vector<AstNode*> aParams(); 
std::vector<AstNode*> aParamsTailOpt(std::vector<AstNode*> args); 
std::vector<VariableDeclarationNode*> fParams(); 
VariableDeclarationNode* param(); 
std::vector<VariableDeclarationNode*> fParamsTailOpt(std::vector<VariableDeclarationNode*> params);


// --- HELPERS (Unchanged) ---
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

// --- PARSER IMPLEMENTATIONS (Updated) ---

AstNode* prog(){ 
    note("prog -> classOrImplOrFuncList"); 
    std::vector<AstNode*> decls = classOrImplOrFuncList(); 
    return new ProgramNode(decls);
}

std::vector<AstNode*> classOrImplOrFuncList(){
    std::vector<AstNode*> decls;
    while(lookahead.type==T_CLASS||lookahead.type==T_IMPLEMENT||lookahead.type==T_FUNC||lookahead.type==T_CONSTRUCT){
        note("classOrImplOrFuncList -> classOrImplOrFunc classOrImplOrFuncList");
        decls.push_back(classOrImplOrFunc());
    }
    note("classOrImplOrFuncList -> epsilon");
    return decls;
}

AstNode* classOrImplOrFunc(){
 if(lookahead.type==T_CLASS){ 
    note("classOrImplOrFunc -> classDecl"); 
    return classDecl(); 
  }
 else if(lookahead.type==T_IMPLEMENT){ 
    note("classOrImplOrFunc -> implDef"); 
    return implDef(); 
  }
 else { 
    note("classOrImplOrFunc -> funcDef"); 
    return funcDef(); 
  }
}

/* classDecl */
AstNode* classDecl(){
 note("classDecl -> class id classDeclOptIsa { classMembers } ;");
 expect(T_CLASS);
  std::string className = lookahead.lexeme;
  expect(T_ID);
 std::vector<IdentifierNode*> superclasses = classDeclOptIsa();
 expect(T_LBRACE); 
  std::vector<AstNode*> members = classMembers(); 
  expect(T_RBRACE); 
  expect(T_SEMI);
  return new ClassDeclarationNode(className, superclasses, members);
}

std::vector<IdentifierNode*> classDeclOptIsa(){
 if(lookahead.type==T_ISA){ 
    note("classDeclOptIsa -> isa id idListTailOpt"); 
    expect(T_ISA);
    std::vector<IdentifierNode*> supers;
    supers.push_back(new IdentifierNode(lookahead.lexeme));
    expect(T_ID); 
    // This is a minimal implementation. idListTailOpt is complex.
    // idListTailOpt(); 
    return supers;
  }
 else {
    note("classDeclOptIsa -> epsilon");
    return std::vector<IdentifierNode*>(); // Return empty vector
  }
}

std::vector<AstNode*> classMembers(){
    std::vector<AstNode*> members;
    while(lookahead.type==T_PUBLIC||lookahead.type==T_PRIVATE){ 
        note("classMembers -> classMember classMembers"); 
        members.push_back(classMember()); 
    }
    note("classMembers -> epsilon");
    return members;
}

AstNode* classMember(){ 
    note("classMember -> visibility memberDecl"); 
    visibility(); // We can pass visibility string to memberDecl if needed
    return memberDecl(); 
}

std::string visibility(){ 
    if(lookahead.type==T_PUBLIC){ 
        note("visibility -> public"); 
        expect(T_PUBLIC);
        return "public";
    } else { 
        note("visibility -> private"); 
        expect(T_PRIVATE);
        return "private";
    } 
}

AstNode* memberDecl(){ 
    if(lookahead.type==T_ATTRIBUTE){ 
        note("memberDecl -> attributeDecl"); 
        return attributeDecl(); 
    } else { 
        note("memberDecl -> funcDecl"); 
        return funcDecl(); 
    } 
}

// funcDecl is just a "header" or "prototype", not a full definition
AstNode* funcDecl(){ 
    note("funcDecl -> funcHead ;"); 
    AstNode* head = funcHead(); 
    expect(T_SEMI); 
    // In a full AST, we might have a distinct FunctionDeclarationNode
    // For now, we return the FunctionDefinitionNode with a null body
    return head;
}

AstNode* funcHead(){
    std::string funcName;
    std::vector<VariableDeclarationNode*> params;
    std::string retType = "void"; // Default for constructor
    bool isCtor = false;

 if(lookahead.type==T_FUNC){
  note("funcHead -> func id ( fParams ) => returnType");
  expect(T_FUNC);
    funcName = lookahead.lexeme;
    expect(T_ID); 
    expect(T_LPAREN); 
    params = fParams(); 
    expect(T_RPAREN); 
    expect(T_ARROW); 
    retType = returnType();
 } else {
  note("funcHead -> constructor ( fParams )");
    isCtor = true;
    funcName = "constructor";
  expect(T_CONSTRUCT); 
    expect(T_LPAREN); 
    params = fParams(); 
    expect(T_RPAREN);
 }
  // Create a FunctionDefinitionNode but with a NULL body (it's just a declaration)
  return new FunctionDefinitionNode(funcName, params, retType, nullptr, isCtor);
}

std::string returnType(){ 
    if(lookahead.type==T_VOID){ 
        note("returnType -> void"); 
        expect(T_VOID);
        return "void";
    } else { 
        note("returnType -> type"); 
        return type(); 
    } 
}

/* implDef */
AstNode* implDef(){ 
    note("implDef -> implement id { funcDefList }"); 
    expect(T_IMPLEMENT); 
    std::string className = lookahead.lexeme;
    expect(T_ID); 
    expect(T_LBRACE); 
    std::vector<AstNode*> funcs = funcDefList(); 
    expect(T_RBRACE); 
    // This is tricky. An implementation isn't a single node.
    // For now, let's just return a placeholder or the first function.
    // A better design might return a vector or a dedicated "ImplNode".
    // Let's just return a "ProgramNode" to hold the list of functions.
    return new ProgramNode(funcs); 
}

std::vector<AstNode*> funcDefList(){ 
    std::vector<AstNode*> funcs;
    while(lookahead.type==T_FUNC||lookahead.type==T_CONSTRUCT){ 
        note("funcDefList -> funcDef funcDefList"); 
        funcs.push_back(funcDef()); 
    } 
    note("funcDefList -> epsilon");
    return funcs;
}

AstNode* funcDef(){ 
    note("funcDef -> funcHead funcBody"); 
    // This is complex. funcHead returns a node, but we need to *modify* it.
    FunctionDefinitionNode* head = (FunctionDefinitionNode*)funcHead();
    head->body = funcBody(); // Now we parse the body and attach it.
    return head;
}

BlockNode* funcBody(){ 
    note("funcBody -> { localVarDeclOrStmtList }"); 
    expect(T_LBRACE); 
    std::vector<AstNode*> stmts = localVarDeclOrStmtList(); 
    expect(T_RBRACE); 
    return new BlockNode(stmts);
}

std::vector<AstNode*> localVarDeclOrStmtList(){ 
    std::vector<AstNode*> stmts;
    while(lookahead.type==T_LOCAL||lookahead.type==T_IF||lookahead.type==T_WHILE||lookahead.type==T_READ||lookahead.type==T_WRITE||lookahead.type==T_RETURN||lookahead.type==T_ID||lookahead.type==T_SELF||lookahead.type==T_LBRACE){ 
        note("localVarDeclOrStmtList -> localVarDeclOrStmt localVarDeclOrStmtList"); 
        stmts.push_back(localVarDeclOrStmt()); 
    } 
    note("localVarDeclOrStmtList -> epsilon");
    return stmts;
}

AstNode* localVarDeclOrStmt(){ 
    if(lookahead.type==T_LOCAL){ 
        note("localVarDeclOrStmt -> localVarDecl"); 
        return localVarDecl(); 
    } else { 
        note("localVarDeclOrStmt -> statement"); 
        return statement(); 
    } 
}

AstNode* localVarDecl(){ 
    note("localVarDecl -> local varDecl"); 
    expect(T_LOCAL); 
    return varDecl(); 
}

AstNode* attributeDecl(){ 
    note("attributeDecl -> attribute varDecl"); 
    expect(T_ATTRIBUTE); 
    return varDecl(); 
}

AstNode* varDecl(){ 
    note("varDecl -> id : type arraySizeList ;"); 
    std::string name = lookahead.lexeme;
    expect(T_ID); 
    expect(T_COLON); 
    std::string typeName = type(); 
    bool isArr = arraySizeList(); 
    expect(T_SEMI); 
    return new VariableDeclarationNode(name, typeName, isArr);
}

bool arraySizeList(){ 
    if(lookahead.type==T_LBRACK){ 
        note("arraySizeList -> arraySize arraySizeList"); 
        arraySize(); // We could parse a list of sizes, but bool is simpler
        // arraySizeList(); // This recursion is tricky, let's just handle one
        return true;
    } else {
        note("arraySizeList -> epsilon");
        return false;
    }
}

bool arraySize(){ 
    note("arraySize -> [ intLit ] | [ ]"); 
    expect(T_LBRACK); 
    if(lookahead.type==T_INTLIT){ 
        expect(T_INTLIT);
    } 
    expect(T_RBRACK); 
    return true;
}

std::string type(){ 
    if(lookahead.type==T_INT_TYPE){ 
        note("type -> integer"); 
        expect(T_INT_TYPE);
        return "integer";
    } else if(lookahead.type==T_FLOAT_TYPE){ 
        note("type -> float"); 
        expect(T_FLOAT_TYPE);
        return "float";
    } else { 
        note("type -> id");
        std::string typeName = lookahead.lexeme;
        expect(T_ID);
        return typeName;
    } 
}

/* statements and blocks */
AstNode* statement(){
  AstNode* node = nullptr;
 if(lookahead.type==T_IF){
  note("statement -> if ( relExpr ) then statBlock else statBlock ;");
  expect(T_IF); 
    expect(T_LPAREN); 
    AstNode* cond = relExpr(); 
    expect(T_RPAREN);
  expect(T_THEN); 
    BlockNode* thenB = statBlock(); 
    expect(T_ELSE); 
    BlockNode* elseB = statBlock(); 
    expect(T_SEMI);
    node = new IfStatementNode(cond, thenB, elseB);
 } else if(lookahead.type==T_WHILE){
  note("statement -> while ( relExpr ) statBlock ;");
  expect(T_WHILE); 
    expect(T_LPAREN); 
    AstNode* cond = relExpr(); 
    expect(T_RPAREN); 
    BlockNode* body = statBlock(); 
    expect(T_SEMI);
    node = new WhileStatementNode(cond, body);
 } else if(lookahead.type==T_READ){
  note("statement -> read ( variable ) ;");
  expect(T_READ); 
    expect(T_LPAREN); 
    AstNode* var = variable(); 
    expect(T_RPAREN); 
    expect(T_SEMI);
    // 'read' is tricky. We'll represent it as a WriteStatement for now
    // A better AST would have a ReadStatementNode
    node = new WriteStatementNode(var); // Placeholder
 } else if(lookahead.type==T_WRITE){
  note("statement -> write ( expr ) ;");
  expect(T_WRITE); 
    expect(T_LPAREN); 
    AstNode* ex = expr(); 
    expect(T_RPAREN); 
    expect(T_SEMI);
    node = new WriteStatementNode(ex);
 } else if(lookahead.type==T_RETURN){
  note("statement -> return ( expr ) ;");
  expect(T_RETURN); 
    expect(T_LPAREN); 
    AstNode* ex = expr(); 
    expect(T_RPAREN); 
    expect(T_SEMI);
    node = new ReturnStatementNode(ex);
 } else if(lookahead.type==T_ID||lookahead.type==T_SELF){
  note("statement -> idOrSelfPrefix stmtAfterIdOrSelf");
    // THIS IS THE NEW, CORRECTED LOGIC
    node = parseIdOrSelfStatement();
  } else if(lookahead.type==T_LBRACE){
    // This is a block used as a statement
    node = statBlock();
 } else if(lookahead.type==T_RBRACE){
  note("statement -> epsilon");
    // Epsilon case (empty statement)
 } else {
  fprintf(stderr, "Unexpected token in statement: %s\n", tokenName(lookahead.type)); exit(1);
 }
  return node;
}

// NEW FUNCTION to handle statements starting with an ID
AstNode* parseIdOrSelfStatement() {
    AstNode* base = variable(); // Parse the complex variable (e.g., self.width[i])
    
    if (lookahead.type == T_ASSIGN) {
        // It's an assignment statement
        return assignOp(base);
    } else if (lookahead.type == T_LPAREN) {
        // It's a function call statement
        note("stmtAfterIdOrSelf -> ( aParams ) ;");
        expect(T_LPAREN);
        std::vector<AstNode*> args = aParams();
        expect(T_RPAREN);
        expect(T_SEMI);
        return new FunctionCallNode(base, args);
    } else {
        fprintf(stderr, "Syntax error: expected := or ( after identifier in statement\n");
        exit(1);
    }
}

AstNode* assignOp(AstNode* var){ 
    note("assignOp -> :="); 
    expect(T_ASSIGN); 
    AstNode* val = expr();
    expect(T_SEMI);
    return new AssignmentNode(var, val);
}

BlockNode* statBlock(){
 if(lookahead.type==T_LBRACE){ 
    note("statBlock -> { statementList }"); 
    expect(T_LBRACE); 
    std::vector<AstNode*> stmts = statementList(); 
    expect(T_RBRACE); 
    return new BlockNode(stmts);
  }
 else if(lookahead.type==T_IF||lookahead.type==T_WHILE||lookahead.type==T_READ||lookahead.type==T_WRITE||lookahead.type==T_RETURN||lookahead.type==T_ID||lookahead.type==T_SELF){ 
    note("statBlock -> statement"); 
    // A single statement can be a "block"
    std::vector<AstNode*> stmts;
    stmts.push_back(statement());
    return new BlockNode(stmts);
  }
 else { 
    note("statBlock -> epsilon"); 
    return new BlockNode(std::vector<AstNode*>()); // Empty block
  }
}

std::vector<AstNode*> statementList(){ 
    std::vector<AstNode*> stmts;
    while(lookahead.type==T_IF||lookahead.type==T_WHILE||lookahead.type==T_READ||lookahead.type==T_WRITE||lookahead.type==T_RETURN||lookahead.type==T_ID||lookahead.type==T_SELF||lookahead.type==T_LBRACE){ 
        note("statementList -> statement statementList"); 
        stmts.push_back(statement()); 
    } 
    note("statementList -> epsilon");
    return stmts;
}

/* expressions: relExpr -> arithExpr [relOp arithExpr] */
AstNode* expr(){ 
    note("expr -> relExpr"); 
    return relExpr(); 
}

AstNode* relExpr(){ 
    note("relExpr -> arithExpr relExpr'"); 
    AstNode* left = arithExpr();
    return relExprPrime(left);
}

AstNode* relExprPrime(AstNode* leftNode){ 
    if(lookahead.type==T_EQ||lookahead.type==T_NEQ||lookahead.type==T_LT||lookahead.type==T_GT||lookahead.type==T_LE||lookahead.type==T_GE){ 
        note("relExpr' -> relOp arithExpr"); 
        std::string op = relOp(); 
        AstNode* right = arithExpr(); 
        AstNode* newNode = new BinaryOpNode(op, leftNode, right);
        // Note: Relational ops are not associative, so we don't recurse
        return newNode; 
    } else {
        note("relExpr' -> epsilon");
        return leftNode;
    }
}

std::string relOp(){ 
    std::string op = lookahead.lexeme;
    if(lookahead.type==T_EQ||lookahead.type==T_NEQ||lookahead.type==T_LT||lookahead.type==T_GT||lookahead.type==T_LE||lookahead.type==T_GE){ 
        note("relOp"); 
        advance(); 
    } else { 
        fprintf(stderr,"relOp expected\n"); exit(1);
    } 
    return op;
}

/* arithExpr -> term arithExpr' ; arithExpr' -> addOp term arithExpr' | epsilon */
AstNode* arithExpr(){ 
    note("arithExpr -> term arithExpr'"); 
    AstNode* left = term(); 
    return arithExprPrime(left);
}

AstNode* arithExprPrime(AstNode* leftNode){ 
    if(lookahead.type==T_PLUS||lookahead.type==T_MINUS||lookahead.type==T_OR){ 
        note("arithExpr' -> addOp term arithExpr'"); 
        std::string op = addOp(); 
        AstNode* right = term(); 
        AstNode* newNode = new BinaryOpNode(op, leftNode, right);
        return arithExprPrime(newNode); // Recurse for associativity
    } else {
        note("arithExpr' -> epsilon");
        return leftNode;
    }
}

std::string addOp(){ 
    std::string op = lookahead.lexeme;
    if(lookahead.type==T_PLUS||lookahead.type==T_MINUS||lookahead.type==T_OR){ 
        note("addOp"); 
        advance(); 
    } else { 
        fprintf(stderr,"addOp expected\n"); exit(1);
    } 
    return op;
}

/* term -> factor term' ; term' -> multOp factor term' | epsilon */
AstNode* term(){ 
    note("term -> factor term'"); 
    AstNode* left = factor();
    return termPrime(left);
}

AstNode* termPrime(AstNode* leftNode){ 
    if(lookahead.type==T_MUL||lookahead.type==T_DIV||lookahead.type==T_AND){ 
        note("term' -> multOp factor term'"); 
        std::string op = multOp(); 
        AstNode* right = factor(); 
        AstNode* newNode = new BinaryOpNode(op, leftNode, right);
        return termPrime(newNode); // Recurse for associativity
    } else {
        note("term' -> epsilon");
        return leftNode;
    }
}

std::string multOp(){ 
    std::string op = lookahead.lexeme;
    if(lookahead.type==T_MUL||lookahead.type==T_DIV||lookahead.type==T_AND){ 
        note("multOp"); 
        advance(); 
    } else { 
        fprintf(stderr,"multOp expected\n"); exit(1);
    }
    return op;
}

/* factor */
AstNode* factor(){
  AstNode* node = nullptr;
 if(lookahead.type==T_INTLIT){ 
    note("factor -> intLit");
    node = new LiteralNode(std::stod(lookahead.lexeme), "integer"); 
    expect(T_INTLIT); 
  }
 else if(lookahead.type==T_FLOATLIT){ 
    note("factor -> floatLit"); 
    node = new LiteralNode(std::stod(lookahead.lexeme), "float");
    expect(T_FLOATLIT); 
  }
 else if(lookahead.type==T_LPAREN){ 
    note("factor -> ( arithExpr )"); 
    expect(T_LPAREN); 
    node = arithExpr(); 
    expect(T_RPAREN); 
  }
 else if(lookahead.type==T_NOT){ 
    note("factor -> not factor"); 
    expect(T_NOT); 
    node = new UnaryOpNode("not", factor()); 
  }
 else if(lookahead.type==T_PLUS||lookahead.type==T_MINUS){ 
    note("factor -> sign factor"); 
    std::string op = lookahead.lexeme;
    sign(); 
    node = new UnaryOpNode(op, factor());
  }
 else if(lookahead.type==T_ID||lookahead.type==T_SELF){ 
    note("factor -> idOrSelfPrefix factorAfterIdOrSelf");
  
    node = variable();
    if (lookahead.type == T_LPAREN) {
       
        note("factorAfterIdOrSelf -> ( aParams )");
        expect(T_LPAREN);
        std::vector<AstNode*> args = aParams();
        expect(T_RPAREN);
        node = new FunctionCallNode(node, args);
    } else {
       
        note("factorAfterIdOrSelf -> variableTail");
        
    }
  }
 else { fprintf(stderr,"factor: unexpected %s\n", tokenName(lookahead.type)); exit(1); }
  return node;
}

AstNode* sign(){ 
    if(lookahead.type==T_PLUS||lookahead.type==T_MINUS){ 
        advance(); 
    } else { 
        fprintf(stderr,"sign expected\n"); exit(1);
    } 
    return nullptr;  
}



AstNode* variable(){ 
    note("variable -> idOrSelfPrefix variableTail");
    
    AstNode* base = nullptr;
    if (lookahead.type == T_ID) {
        base = new IdentifierNode(lookahead.lexeme);
        expect(T_ID);
    } else if (lookahead.type == T_SELF) {
        base = new IdentifierNode("self");
        expect(T_SELF);
    } else {
        fprintf(stderr, "Expected id or self in variable\n"); exit(1);
    }
    
    
    return parseVariableAccess(base);
}

AstNode* parseVariableAccess(AstNode* base) {
    if (lookahead.type == T_LBRACK) {
        note("indiceList -> indice indiceList");
        AstNode* indexExpr = indice();
        AstNode* accessNode = new ArrayAccessNode(base, indexExpr);
        return parseVariableAccess(accessNode); 
    } else if (lookahead.type == T_DOT) {
       
        advance();
        AstNode* member = new IdentifierNode(lookahead.lexeme);
        expect(T_ID);
        AstNode* accessNode = new BinaryOpNode(".", base, member);
        return parseVariableAccess(accessNode); 
    } else {
        note("indiceList -> epsilon");
        return base; 
    }
}

AstNode* indice(){ 
    note("indice -> [ arithExpr ]"); 
    expect(T_LBRACK); 
    AstNode* expr = arithExpr(); 
    expect(T_RBRACK); 
    return expr;
}


std::vector<AstNode*> aParams(){ 
    std::vector<AstNode*> args;
    if(lookahead.type==T_LPAREN||lookahead.type==T_NOT||lookahead.type==T_PLUS||lookahead.type==T_MINUS||lookahead.type==T_INTLIT||lookahead.type==T_FLOATLIT||lookahead.type==T_ID||lookahead.type==T_SELF){ 
        note("aParams -> expr aParamsTailOpt"); 
        args.push_back(expr()); 
        return aParamsTailOpt(args); 
    } else {
        note("aParams -> epsilon");
        return args; 
    }
}

std::vector<AstNode*> aParamsTailOpt(std::vector<AstNode*> args){ 
    if(lookahead.type==T_COMMA){ 
        note("aParamsTailOpt -> , expr aParamsTailOpt"); 
        expect(T_COMMA); 
        args.push_back(expr());
        return aParamsTailOpt(args); 
    } else {
        note("aParamsTailOpt -> epsilon");
        return args;
    }
}

std::vector<VariableDeclarationNode*> fParams(){ 
    std::vector<VariableDeclarationNode*> params;
    if(lookahead.type==T_ID){ 
        note("fParams -> param fParamsTailOpt"); 
        params.push_back(param()); 
        return fParamsTailOpt(params); 
    } else {
        note("fParams -> epsilon");
        return params; 
    }
}

VariableDeclarationNode* param(){ 
    note("param -> id : type arraySizeList"); 
    std::string name = lookahead.lexeme;
    expect(T_ID); 
    expect(T_COLON); 
    std::string typeName = type(); 
    bool isArr = arraySizeList();
    return new VariableDeclarationNode(name, typeName, isArr); 
}

std::vector<VariableDeclarationNode*> fParamsTailOpt(std::vector<VariableDeclarationNode*> params){ 
    if(lookahead.type==T_COMMA){ 
        note("fParamsTailOpt -> , param fParamsTailOpt"); 
        expect(T_COMMA); 
        params.push_back(param()); 
        return fParamsTailOpt(params); 
    } else {
        note("fParamsTailOpt -> epsilon");
        return params;
    }
}



int main(int argc, char** argv){
 if(argc<2){ fprintf(stderr, "usage: %s <source-file>\n", argv[0]); return 1; }

 FILE* src = fopen(argv[1], "r");
 if(!src){ perror("fopen"); return 1; }

 extern FILE* yyin;
 yyin = src;

 DERIV = fopen("derivation.txt","w");
 if(!DERIV){ perror("deriv open"); return 1; }

 advance();      
 note("Start Derivation:");
 
 
  AstNode* astRoot = prog(); 
 
  if(lookahead.type != T_EOF){
  fprintf(stderr,"Extra input after program: %s\n", lookahead.lexeme.c_str());
    fclose(DERIV);
    fclose(src);
  return 1;
  }
  
  if (astRoot == nullptr) {
      fprintf(stderr, "Syntax Error: AST creation failed.\n");
      fclose(DERIV);
      fclose(src);
      return 1;
  }

  printf("OK: syntax correct. AST created. Derivation written to derivation.txt\n");
  
  
  printf("Running semantic analysis...\n");
  SemanticAnalyzer analyzer;
//   bool semanticOK = analyzer.analyze(astRoot);

 if (analyzer.analyze(astRoot)) {
        printf("2. Semantic Analysis: Passed.\n");
        
        // 4. Code Generation
        // We pass the SymbolTable from the analyzer because it has the offsets!
        CodeGen generator(analyzer.getTable()); 
        
        if (generator.generate(astRoot, "out.s")) {
            printf("3. Code Generation: Passed. Assembly written to 'out.s'.\n");
        } else {
            fprintf(stderr, "Code Generation Failed: Could not open output file.\n");
            return 1;
        }

    } else {
        fprintf(stderr, "Semantic Analysis Failed.\n");
        return 1;
    }
  // --- CLEANUP ---
  //TODO:  delete the AST to prevent memory leaks
  
  
 fclose(DERIV);
 fclose(src);
 return (semanticOK ? 0 : 1); 
}