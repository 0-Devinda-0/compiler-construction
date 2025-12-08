#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include "ast.h"
#include "SymbolTable.h"
#include <iostream>
#include <string>

/**
 * @class SemanticAnalyzer
 * @brief Walks the AST to build the Symbol Table and find semantic errors.
 *
 * This class finds errors like type mismatches, undeclared variables,
 * and function re-declarations.
 */
class SemanticAnalyzer {
private:
    SymbolTable table; // The symbol table
    bool hasError;     // Flag to track if any errors occurred

    int currentLocalOffset; // Tracks the stack offset for local variables
    // --- Private Helper Functions ---

    /**
     * @brief The main "visitor" for all nodes that are statements (actions).
     * It dispatches to the correct private helper (e.g., checkProgram, checkAssignment).
     */
    void checkStatement(AstNode* node);

    /**
     * @brief The main "visitor" for all nodes that are expressions (have a value).
     * It dispatches to the correct private helper and returns the expression's data type
     * as a string (e.g., "integer", "float", "MyClass", "error_type").
     */
    std::string getExpressionType(AstNode* node);

    // --- Statement Checkers (void) ---
    void checkProgram(ProgramNode* node);
    void checkClassDeclaration(ClassDeclarationNode* node);
    void checkFunctionDefinition(FunctionDefinitionNode* node);
    void checkVariableDeclaration(VariableDeclarationNode* node);
    void checkBlock(BlockNode* node);
    void checkAssignment(AssignmentNode* node);
    void checkIf(IfStatementNode* node);
    void checkWhile(WhileStatementNode* node);
    void checkWrite(WriteStatementNode* node);
    void checkReturn(ReturnStatementNode* node);
    void checkFunctionCallStatement(FunctionCallNode* node); 

    // --- Expression Type-Getters (return std::string) ---
    std::string getLiteralType(LiteralNode* node);
    std::string getIdentifierType(IdentifierNode* node);
    std::string getBinaryOpType(BinaryOpNode* node);
    std::string getUnaryOpType(UnaryOpNode* node);
    std::string getFunctionCallType(FunctionCallNode* node); 
    std::string getArrayAccessType(ArrayAccessNode* node);

    // --- Utility ---
    
    /**
     * @brief Prints a semantic error message to stderr.
     * @param message The error message.
     */
    void reportError(const std::string& message) {
        hasError = true;
        std::cerr << "Semantic Error: " << message << std::endl;
    }

public:
    /**
     * @brief Constructor. Initializes the error flag.
     */
    SemanticAnalyzer() : hasError(false) {}

    /**
     * @brief The main public entry point to start semantic analysis.
     * @param root The root node of the AST (a ProgramNode).
     * @return True if analysis passes with no errors, false otherwise.
     */
    bool analyze(AstNode* root) {
        checkStatement(root); // Start the traversal at the root
        return !hasError;
    }
};

#endif // SEMANTICANALYZER_H