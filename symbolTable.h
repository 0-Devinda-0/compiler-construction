#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <map>
#include <vector>
#include "ast.h" 


struct SymbolEntry {
    std::string name;             
    std::string type;             
    std::string kind;
    int offset; // the memory offset             
    AstNode* declarationNode; 
    
    
};

class SymbolTable {
private:
   
    using Scope = std::map<std::string, SymbolEntry>;

    std::vector<Scope> scopeStack;

public:
   
    SymbolTable() {
        enterScope(); 
    }

  
    void enterScope() {
        scopeStack.push_back(Scope());
    }

 
    void exitScope() {
        if (!scopeStack.empty()) {
            scopeStack.pop_back();
        }
    }

 
    bool insert(const SymbolEntry& entry) {
        if (scopeStack.empty()) {
            return false; 
        }
        
        Scope& currentScope = scopeStack.back();
        
      
        if (currentScope.count(entry.name)) {
            return false; 
        }
        
        currentScope[entry.name] = entry;
        return true;
    }

    
    SymbolEntry* lookup(const std::string& name) {
        
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
            Scope& scope = *it;
            if (scope.count(name)) {
                
                return &scope[name];
            }
        }
        return nullptr; 
    }
};

#endif