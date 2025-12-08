#include "codeGen.h"
#include <iostream>

void CodeGen::genStatement(AstNode* node) {
    if (!node) return;
    if (auto* p = dynamic_cast<ProgramNode*>(node)) genProgram(p);
    else if (auto* p = dynamic_cast<FunctionDefinitionNode*>(node)) genFunction(p);
    else if (auto* p = dynamic_cast<BlockNode*>(node)) genBlock(p);
    else if (auto* p = dynamic_cast<AssignmentNode*>(node)) genAssignment(p);
    else if (auto* p = dynamic_cast<IfStatementNode*>(node)) genIf(p);
    else if (auto* p = dynamic_cast<WhileStatementNode*>(node)) genWhile(p);
    else if (auto* p = dynamic_cast<WriteStatementNode*>(node)) genWrite(p);
    else if (auto* p = dynamic_cast<ReturnStatementNode*>(node)) genReturn(p);
    // ... add others ...
}

void CodeGen::genExpression(AstNode* node) {
    if (!node) return;
    if (auto* p = dynamic_cast<LiteralNode*>(node)) genLiteral(p);
    else if (auto* p = dynamic_cast<IdentifierNode*>(node)) genIdentifier(p);
    else if (auto* p = dynamic_cast<BinaryOpNode*>(node)) genBinaryOp(p);
    // ... add others ...
}

// --- High Level Structures ---

void CodeGen::genProgram(ProgramNode* node) {
    outFile << ".text\n";
    for (AstNode* decl : node->declarations) {
        genStatement(decl);
    }
}

void CodeGen::genFunction(FunctionDefinitionNode* node) {
    // 1. Function Label
    outFile << node->name << ":\n";
    
    // 2. Prologue (Setup Stack Frame)
    outFile << "  push rbp\n";
    outFile << "  mov rbp, rsp\n";
    // Reserve space for locals (simple version: subtract 128 bytes just to be safe)
    outFile << "  sub rsp, 128\n"; 
    
    // 3. Body
    // We need to ENTER the function scope to see the variable offsets!
    table.enterScope(); 
    // (Note: In a real implementation, you'd need to re-populate params here
    // or keep the table state from the semantic phase. 
    // Ideally, the AST nodes themselves should store the offsets now.)
    
    genStatement(node->body);

    table.exitScope();

    // 4. Epilogue (Cleanup)
    // Note: 'ret' is handled by genReturn, but we add a fallback here for void funcs
    outFile << "  leave\n"; // Equivalent to mov rsp, rbp; pop rbp
    outFile << "  ret\n\n";
}

void CodeGen::genBlock(BlockNode* node) {
    for (AstNode* stmt : node->statements) {
        genStatement(stmt);
    }
}

// --- Statements ---

void CodeGen::genAssignment(AssignmentNode* node) {
    // 1. Calculate the value (result ends up in RAX)
    genExpression(node->value);
    
    // 2. Find the target variable's offset
    // (Assuming target is a simple IdentifierNode for now)
    if (auto* id = dynamic_cast<IdentifierNode*>(node->target)) {
        SymbolEntry* entry = table.lookup(id->name);
        if (entry) {
            outFile << "  mov [rbp" << (entry->offset >= 0 ? "+" : "") << entry->offset << "], rax\n";
        }
    }
}

void CodeGen::genReturn(ReturnStatementNode* node) {
    if (node->expression) {
        genExpression(node->expression); // Result in RAX
    }
    outFile << "  leave\n";
    outFile << "  ret\n";
}

void CodeGen::genWrite(WriteStatementNode* node) {
    // We will use printf("%ld\n", value)
    
    // 1. Calculate value (in RAX)
    genExpression(node->expression);
    
    // 2. Setup arguments for printf (System V AMD64 ABI)
    outFile << "  mov rsi, rax\n";       // 2nd argument: value
    outFile << "  lea rdi, .LC_PRINT_INT[rip]\n"; // 1st argument: format string
    outFile << "  mov eax, 0\n";         // No vector registers used
    outFile << "  call printf@PLT\n";
    
    // Note: You need to add .LC_PRINT_INT to the data section!
}

// --- Expressions (The Stack Machine) ---

void CodeGen::genLiteral(LiteralNode* node) {
    outFile << "  mov rax, " << (int)node->value << "\n";
}

void CodeGen::genIdentifier(IdentifierNode* node) {
    SymbolEntry* entry = table.lookup(node->name);
    if (entry) {
        outFile << "  mov rax, [rbp" << (entry->offset >= 0 ? "+" : "") << entry->offset << "]\n";
    }
}

void CodeGen::genBinaryOp(BinaryOpNode* node) {
    // 1. Evaluate Left -> RAX
    genExpression(node->left);
    
    // 2. Push Left to Stack
    outFile << "  push rax\n";
    
    // 3. Evaluate Right -> RAX
    genExpression(node->right);
    
    // 4. Pop Left -> RBX
    outFile << "  pop rbx\n";
    
    // 5. Perform Operation (Result -> RAX)
    // Note: Order is 'op left(rbx), right(rax)'
    
    if (node->op == "+") {
        outFile << "  add rax, rbx\n"; // This adds right to left? No, add is commutative.
        // Wait, 'add rbx, rax' puts result in rbx. We want result in rax.
        // Better: 'add rax, rbx' -> rax = rax + rbx.
    } else if (node->op == "-") {
        // We want: rbx - rax
        outFile << "  sub rbx, rax\n";
        outFile << "  mov rax, rbx\n";
    } else if (node->op == "*") {
        outFile << "  imul rax, rbx\n";
    }
    // (Add division and logic ops here)
}

// In CodeGen.cpp

void CodeGen::genIf(IfStatementNode* node) {
    std::string elseLabel = newLabel();
    std::string endIfLabel = newLabel();

    // 1. Evaluate the condition (result in RAX)
    genExpression(node->condition);

    // 2. Compare the result (RAX) to 0 (False)
    outFile << "  cmp rax, 0\n";
    
    // 3. Conditional Jump: If (RAX == 0) (False), jump to ELSE
    outFile << "  je " << elseLabel << "\n";

    // 4. THEN Block
    genBlock(node->thenBranch);
    
    // 5. Unconditional jump over the ELSE block
    outFile << "  jmp " << endIfLabel << "\n";

    // 6. ELSE Label
    outFile << elseLabel << ":\n";
    if (node->elseBranch) {
        genBlock(node->elseBranch);
    }


    // 7. ENDIF Label
    outFile << endIfLabel << ":\n";
}

// In CodeGen.cpp

void CodeGen::genWhile(WhileStatementNode* node) {
    std::string startLoopLabel = newLabel();
    std::string endLoopLabel = newLabel();
    
    // 1. START Loop Label
    outFile << startLoopLabel << ":\n";

    // 2. Evaluate Condition (result in RAX)
    genExpression(node->condition);

    // 3. Compare the result (RAX) to 0 (False)
    outFile << "  cmp rax, 0\n";
    
    // 4. Conditional Jump: If (RAX == 0) (False), jump to END
    outFile << "  je " << endLoopLabel << "\n";

    // 5. Loop Body
    genBlock(node->body);
    
    // 6. Unconditional jump back to the START
    outFile << "  jmp " << startLoopLabel << "\n";

    // 7. END Loop Label
    outFile << endLoopLabel << ":\n";
}

// In CodeGen.cpp (Inside genBinaryOp)

// After Step 4 (Pop Left -> RBX, Right in RAX):
// Now: Left is in RBX, Right is in RAX. We want RBX compared to RAX.

if (node->op == "==" || node->op == "<" || node->op == ">") {
    // 1. Compare Left (RBX) with Right (RAX)
    outFile << "  cmp rbx, rax\n";

    // 2. Use SET instructions to get 0 or 1. Result goes into the AL register (lower 8 bits of RAX)
    if (node->op == "==") {
        outFile << "  sete al\n"; // Set AL to 1 if Equal
    } else if (node->op == "<") {
        outFile << "  setl al\n"; // Set AL to 1 if Less
    } else if (node->op == ">") {
        outFile << "  setg al\n"; // Set AL to 1 if Greater
    }
    // (Add logic for <=, >=, and != here...)

    // 3. Extend the result to the full 64-bit RAX register
    // This clears the upper bits, ensuring RAX is either 0 or 1.
    outFile << "  movzx rax, al\n"; 

} else if (node->op == "+" || node->op == "-" || node->op == "*") {
    // ... (Your arithmetic logic from before) ...
}

// ... rest of genBinaryOp

// In CodeGen.cpp (Inside genBinaryOp)

// After Step 4 (Pop Left -> RBX, Right in RAX):
// Now: Left is in RBX, Right is in RAX. We want RBX compared to RAX.

if (node->op == "==" || node->op == "<" || node->op == ">") {
    // 1. Compare Left (RBX) with Right (RAX)
    outFile << "  cmp rbx, rax\n";

    // 2. Use SET instructions to get 0 or 1. Result goes into the AL register (lower 8 bits of RAX)
    if (node->op == "==") {
        outFile << "  sete al\n"; // Set AL to 1 if Equal
    } else if (node->op == "<") {
        outFile << "  setl al\n"; // Set AL to 1 if Less
    } else if (node->op == ">") {
        outFile << "  setg al\n"; // Set AL to 1 if Greater
    }
    // (Add logic for <=, >=, and != here...)

    // 3. Extend the result to the full 64-bit RAX register
    // This clears the upper bits, ensuring RAX is either 0 or 1.
    outFile << "  movzx rax, al\n"; 

} else if (node->op == "+" || node->op == "-" || node->op == "*") {
    // ... (Your arithmetic logic from before) ...
}
