#include "SemanticAnalyzer.h"

// --- Main Dispatcher Functions ---

void SemanticAnalyzer::checkStatement(AstNode* node) {
    if (!node) return;
    
    if (auto* p = dynamic_cast<ProgramNode*>(node)) {
        checkProgram(p);
    } 
    // --- DECLARATIONS ---
    else if (auto* p = dynamic_cast<ClassDeclarationNode*>(node)) {
        checkClassDeclaration(p);
    } else if (auto* p = dynamic_cast<FunctionDefinitionNode*>(node)) {
        checkFunctionDefinition(p);
    } else if (auto* p = dynamic_cast<VariableDeclarationNode*>(node)) {
        checkVariableDeclaration(p);
    } 
    // --- STATEMENTS ---
    else if (auto* p = dynamic_cast<BlockNode*>(node)) {
        checkBlock(p);
    } else if (auto* p = dynamic_cast<AssignmentNode*>(node)) {
        checkAssignment(p);
    } else if (auto* p = dynamic_cast<IfStatementNode*>(node)) {
        checkIf(p);
    } else if (auto* p = dynamic_cast<WhileStatementNode*>(node)) {
        checkWhile(p);
    } else if (auto* p = dynamic_cast<WriteStatementNode*>(node)) {
        checkWrite(p);
    } else if (auto* p = dynamic_cast<ReturnStatementNode*>(node)) {
        checkReturn(p);
    } else if (auto* p = dynamic_cast<FunctionCallNode*>(node)) {
        checkFunctionCallStatement(p);
    } else {
        reportError("Unknown statement node in semantic analysis.");
    }
}

std::string SemanticAnalyzer::getExpressionType(AstNode* node) {
    if (!node) return "void"; 

    // dynamic_cast to find the expression's type and call the correct helper.
    if (auto* p = dynamic_cast<LiteralNode*>(node)) {
        return getLiteralType(p);
    } else if (auto* p = dynamic_cast<IdentifierNode*>(node)) {
        return getIdentifierType(p);
    } else if (auto* p = dynamic_cast<BinaryOpNode*>(node)) {
        return getBinaryOpType(p);
    } else if (auto* p = dynamic_cast<UnaryOpNode*>(node)) {
        return getUnaryOpType(p);
    } else if (auto* p = dynamic_cast<FunctionCallNode*>(node)) {
        return getFunctionCallType(p);
    } else if (auto* p = dynamic_cast<ArrayAccessNode*>(node)) {
        return getArrayAccessType(p);
    } else {
        reportError("Unknown expression node in semantic analysis.");
        return "error_type";
    }
}

// --- Statement Checker Implementations ---

void SemanticAnalyzer::checkProgram(ProgramNode* node) {
    
    for (AstNode* decl : node->declarations) {
        checkStatement(decl);
    }
}

void SemanticAnalyzer::checkClassDeclaration(ClassDeclarationNode* node) {
    SymbolEntry entry;
    entry.name = node->name;
    entry.type = "class"; 
    entry.kind = "class";
    entry.declarationNode = node;
    
    if (!table.insert(entry)) {
        reportError("Redeclaration of class '" + node->name + "'");
    }
    
    table.enterScope();
    for (AstNode* member : node->members) {
        checkStatement(member);
    }
    table.exitScope();
}

void SemanticAnalyzer::checkFunctionDefinition(FunctionDefinitionNode* node) {
    SymbolEntry entry;
    entry.name = node->name;
    entry.type = node->returnTypeName;
    entry.kind = "function";
    entry.declarationNode = node;
    
    if (!table.insert(entry)) {
        reportError("Redeclaration of function '" + node->name + "'");
    }
    
    table.enterScope();

    currentLocalOffset = -8; 
    int paramOffset = 16;
    
    for (VariableDeclarationNode* param : node->parameters) {
        // checkVariableDeclaration(param); 
        SymbolEntry entry;
        entry.name = param->name;
        entry.type = param->typeName;
        entry.kind = "parameter";
        entry.declarationNode = param;
        entry.offset = paramOffset; 
        
        table.insert(entry);
        paramOffset += 8;
    }
    
   
    if (node->body != nullptr) {
        checkBlock(node->body);
    }
    
    table.exitScope();
}

void SemanticAnalyzer::checkVariableDeclaration(VariableDeclarationNode* node) {
    SymbolEntry entry;
    entry.name = node->name;
    entry.type = node->typeName;
    entry.kind = "variable";
    entry.declarationNode = node;
    
    entry.offset = currentLocalOffset;
    currentLocalOffset -= 8;

    if (!table.insert(entry)) {
        reportError("Redeclaration of variable '" + node->name + "'");
    }
}

void SemanticAnalyzer::checkBlock(BlockNode* node) {
 
    if (!node) {
        return; 
    }
    
   
    for (AstNode* stmt : node->statements) {
        checkStatement(stmt);
    }
}

void SemanticAnalyzer::checkAssignment(AssignmentNode* node) {
    std::string targetType = getExpressionType(node->target);
    std::string valueType = getExpressionType(node->value);
    
    if (targetType == "error_type" || valueType == "error_type") {
        return; 
    }
    
    if (targetType == "float" && valueType == "integer") {
       
    } else if (targetType != valueType) {
        reportError("Type mismatch: Cannot assign type '" + valueType + "' to variable of type '" + targetType + "'.");
    }
}

void SemanticAnalyzer::checkIf(IfStatementNode* node) {
    std::string condType = getExpressionType(node->condition);
  
    table.enterScope();
    checkBlock(node->thenBranch);
    table.exitScope();
    
    if (node->elseBranch) {
      
        table.enterScope();
        checkBlock(node->elseBranch);
        table.exitScope();
    }
}

void SemanticAnalyzer::checkWhile(WhileStatementNode* node) {
    std::string condType = getExpressionType(node->condition);
 
    table.enterScope();
    checkBlock(node->body);
    table.exitScope();
}

void SemanticAnalyzer::checkWrite(WriteStatementNode* node) {
    getExpressionType(node->expression); 
}

void SemanticAnalyzer::checkReturn(ReturnStatementNode* node) {
    getExpressionType(node->expression); 
}

void SemanticAnalyzer::checkFunctionCallStatement(FunctionCallNode* node) {
    getFunctionCallType(node); 
}

// --- Expression Type-Getter Implementations ---

std::string SemanticAnalyzer::getLiteralType(LiteralNode* node) {
  
    return node->literalType;
}

std::string SemanticAnalyzer::getIdentifierType(IdentifierNode* node) {
    SymbolEntry* entry = table.lookup(node->name);
    
    if (!entry) {
        reportError("Use of undeclared identifier '" + node->name + "'");
        return "error_type";
    }
    
    return entry->type;
}

std::string SemanticAnalyzer::getBinaryOpType(BinaryOpNode* node) {
    std::string leftType = getExpressionType(node->left);
    std::string rightType = getExpressionType(node->right);
    
    if (leftType == "error_type" || rightType == "error_type") {
        return "error_type";
    }
    
    
    if (node->op == "==" || node->op == "<>" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=" || node->op == "and" || node->op == "or") {
        if (leftType != rightType && !( (leftType == "integer" && rightType == "float") || (leftType == "float" && rightType == "integer") ) ) {
             reportError("Type mismatch: Cannot compare types '" + leftType + "' and '" + rightType + "'.");
             return "error_type";
        }
        return "integer"; 
    }

    
    if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/") {
        if (leftType == "integer" && rightType == "integer") {
            return "integer";
        }
        if ( (leftType == "integer" && rightType == "float") || 
             (leftType == "float" && rightType == "integer") || 
             (leftType == "float" && rightType == "float") ) {
            return "float"; 
        }
        reportError("Type mismatch: Cannot perform '" + node->op + "' on types '" + leftType + "' and '" + rightType + "'.");
        return "error_type";
    }
    
    reportError("Unknown binary operator '" + node->op + "'");
    return "error_type";
}

std::string SemanticAnalyzer::getUnaryOpType(UnaryOpNode* node) {
    std::string exprType = getExpressionType(node->expression);
    if (exprType == "error_type") {
        return "error_type";
    }
    
    if (node->op == "-" || node->op == "+") {
        if (exprType != "integer" && exprType != "float") {
            reportError("Type mismatch: Unary '" + node->op + "' cannot be applied to type '" + exprType + "'.");
            return "error_type";
        }
        return exprType;
    }
    
    if (node->op == "not") {
        if (exprType != "integer") {
            reportError("Type mismatch: 'not' operator cannot be applied to type '" + exprType + "'.");
            return "error_type";
        }
        return "integer";
    }
    
    return "error_type";
}

std::string SemanticAnalyzer::getFunctionCallType(FunctionCallNode* node) {
    std::string funcName;
    if (auto* p = dynamic_cast<IdentifierNode*>(node->functionName)) {
        funcName = p->name;
    } else {
        reportError("Can only call functions by name.");
        return "error_type";
    }

    SymbolEntry* entry = table.lookup(funcName);
    if (!entry) {
        reportError("Call to undeclared function '" + funcName + "'");
        return "error_type";
    }
    if (entry->kind != "function") {
        reportError("'" + funcName + "' is not a function.");
        return "error_type";
    }
    
    FunctionDefinitionNode* funcDecl = static_cast<FunctionDefinitionNode*>(entry->declarationNode);
    if (funcDecl->parameters.size() != node->arguments.size()) {
        reportError("Incorrect number of arguments for function '" + funcName + "'");
    }
    
    for (size_t i = 0; i < node->arguments.size() && i < funcDecl->parameters.size(); ++i) {
        std::string paramType = funcDecl->parameters[i]->typeName;
        std::string argType = getExpressionType(node->arguments[i]);
        
        if (argType == "error_type") continue;

        if (paramType == "float" && argType == "integer") {
             
        } else if (paramType != argType) {
            reportError("Type mismatch in argument " + std::to_string(i+1) + " of function '" + funcName + "': Expected '" + paramType + "', got '" + argType + "'.");
        }
    }
    
    return entry->type;
}

std::string SemanticAnalyzer::getArrayAccessType(ArrayAccessNode* node) {
    std::string arrayType = getExpressionType(node->arrayIdentifier);
    
    std::string indexType = getExpressionType(node->indexExpression);
    if (indexType != "integer" && indexType != "error_type") {
        reportError("Array index must be an integer.");
    }
    
    if (arrayType == "error_type") {
        return "error_type";
    }
   
    return arrayType; 
}