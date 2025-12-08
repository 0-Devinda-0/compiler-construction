#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "SymbolTable.h"
#include <fstream>
#include <string>

class CodeGen {
private:
    std::ofstream outFile;
    SymbolTable& table; // Reference to the symbol table (now populated with offsets)
    int labelCounter;   // For generating unique labels (L1, L2, etc.)

    // Helper to generate unique labels
    std::string newLabel() {
        return ".L" + std::to_string(labelCounter++);
    }

    // --- Visitor Methods ---
    void genStatement(AstNode* node);
    void genExpression(AstNode* node); // Results always end up in RAX

    // Specific Generators
    void genProgram(ProgramNode* node);
    void genFunction(FunctionDefinitionNode* node);
    void genBlock(BlockNode* node);
    void genAssignment(AssignmentNode* node);
    void genIf(IfStatementNode* node);
    void genWhile(WhileStatementNode* node);
    void genWrite(WriteStatementNode* node);
    void genReturn(ReturnStatementNode* node);
    
    void genBinaryOp(BinaryOpNode* node);
    void genLiteral(LiteralNode* node);
    void genIdentifier(IdentifierNode* node);

public:
    CodeGen(SymbolTable& symTable) : table(symTable), labelCounter(0) {}

    bool generate(AstNode* root, const std::string& filename) {
        outFile.open(filename);
        if (!outFile.is_open()) return false;

        // Standard Assembly Header
        outFile << ".intel_syntax noprefix\n"; // Use Intel syntax (mov eax, 1)
        outFile << ".global main\n";           // Make 'main' visible to linker
        outFile << ".extern printf\n\n";       // Use C library printf

        // Generate Code
        genStatement(root);

        outFile.close();
        return true;
    }
};

#endif // CODEGEN_H