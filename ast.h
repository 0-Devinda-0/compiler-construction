#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory> 

// --- Base Class ---

class AstNode {
public:
    virtual ~AstNode() = default;
};

// --- Forward Declarations for Node Types ---
class BlockNode;
class VariableDeclarationNode;
class IdentifierNode;

// --- Expression Nodes ---


class LiteralNode : public AstNode {
public:
    std::string literalType;
    double value;
    LiteralNode(double val,const std::string& type) : value(val),literalType(type) {}
};

class IdentifierNode : public AstNode {
public:
    std::string name;
    IdentifierNode(const std::string& n) : name(n) {}
};

class BinaryOpNode : public AstNode {
public:
    std::string op;
    AstNode* left;
    AstNode* right;
    BinaryOpNode(const std::string& o, AstNode* l, AstNode* r) : op(o), left(l), right(r) {}
};

class UnaryOpNode : public AstNode {
public:
    std::string op;
    AstNode* expression;
    UnaryOpNode(const std::string& o, AstNode* expr) : op(o), expression(expr) {}
};

class FunctionCallNode : public AstNode {
public:
    AstNode* functionName; // This is an expression, often just an IdentifierNode
    std::vector<AstNode*> arguments;
    FunctionCallNode(AstNode* name, const std::vector<AstNode*>& args) : functionName(name), arguments(args) {}
};

class ArrayAccessNode : public AstNode {
public:
    AstNode* arrayIdentifier;
    AstNode* indexExpression;
    ArrayAccessNode(AstNode* arr, AstNode* idx) : arrayIdentifier(arr), indexExpression(idx) {}
};


// --- Statement Nodes ---


class AssignmentNode : public AstNode {
public:
    AstNode* target; 
    AstNode* value;  
    AssignmentNode(AstNode* t, AstNode* v) : target(t), value(v) {}
};

class IfStatementNode : public AstNode {
public:
    AstNode* condition;
    BlockNode* thenBranch;
    BlockNode* elseBranch; 
    IfStatementNode(AstNode* cond, BlockNode* thenB, BlockNode* elseB) : condition(cond), thenBranch(thenB), elseBranch(elseB) {}
};

class WhileStatementNode : public AstNode {
public:
    AstNode* condition;
    BlockNode* body;
    WhileStatementNode(AstNode* cond, BlockNode* b) : condition(cond), body(b) {}
};

class WriteStatementNode : public AstNode {
public:
    AstNode* expression;
    WriteStatementNode(AstNode* expr) : expression(expr) {}
};

class ReturnStatementNode : public AstNode {
public:
    AstNode* expression; 
    ReturnStatementNode(AstNode* expr) : expression(expr) {}
};

class BlockNode : public AstNode {
public:
    std::vector<AstNode*> statements;
    BlockNode(const std::vector<AstNode*>& stmts) : statements(stmts) {}
};


// --- Declaration Nodes ---

class VariableDeclarationNode : public AstNode {
public:
    std::string name;
    std::string typeName;
    bool isArray;
    VariableDeclarationNode(const std::string& n, const std::string& t, bool arr = false) : name(n), typeName(t), isArray(arr) {}
};

class FunctionDefinitionNode : public AstNode {
public:
    std::string name;
    std::vector<VariableDeclarationNode*> parameters;
    std::string returnTypeName; 
    BlockNode* body;
    bool isConstructor;
    FunctionDefinitionNode(const std::string& n, const std::vector<VariableDeclarationNode*>& params, const std::string& retType, BlockNode* b, bool isCtor = false)
        : name(n), parameters(params), returnTypeName(retType), body(b), isConstructor(isCtor) {}
};

class ClassDeclarationNode : public AstNode {
public:
    std::string name;
    std::vector<IdentifierNode*> superclasses;
    std::vector<AstNode*> members; 
    ClassDeclarationNode(const std::string& n, const std::vector<IdentifierNode*>& supers, const std::vector<AstNode*>& mems)
        : name(n), superclasses(supers), members(mems) {}
};


// --- Root Node ---


class ProgramNode : public AstNode {
public:
    std::vector<AstNode*> declarations; 
    ProgramNode(const std::vector<AstNode*>& decls) : declarations(decls) {}
};


#endif 