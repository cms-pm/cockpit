#include "symbol_table.h"
#include <iostream>
#include <algorithm>

SymbolTable::SymbolTable() 
    : currentScope(0), nextGlobalIndex(0), currentStackOffset(0) {
    initializeBuiltins();
}

void SymbolTable::enterScope() {
    currentScope++;
    // Stack offset continues from previous scope
}

void SymbolTable::exitScope() {
    if (currentScope > 0) {
        // Remove symbols from current scope
        symbols.erase(
            std::remove_if(symbols.begin(), symbols.end(),
                [this](const Symbol& s) { return s.scopeDepth >= currentScope; }),
            symbols.end()
        );
        currentScope--;
    }
}

bool SymbolTable::declareSymbol(const std::string& name, SymbolType type, DataType dataType) {
    // Check if symbol already exists in current scope
    for (const auto& symbol : symbols) {
        if (symbol.name == name && symbol.scopeDepth == currentScope) {
            return false; // Symbol already declared in this scope
        }
    }
    
    Symbol newSymbol(name, type, dataType, currentScope);
    
    // Allocate memory based on scope
    if (currentScope == 0) {
        // Global variable
        newSymbol.globalIndex = allocateGlobal();
        newSymbol.isGlobal = true;
    } else {
        // Local variable
        newSymbol.stackOffset = allocateLocal();
        newSymbol.isGlobal = false;
    }
    
    symbols.push_back(newSymbol);
    return true;
}

Symbol* SymbolTable::lookupSymbol(const std::string& name) {
    // Search from most recent scope to global scope
    for (auto it = symbols.rbegin(); it != symbols.rend(); ++it) {
        if (it->name == name && it->scopeDepth <= currentScope) {
            return &(*it);
        }
    }
    return nullptr;
}

bool SymbolTable::isSymbolDeclared(const std::string& name) const {
    return std::any_of(symbols.begin(), symbols.end(),
        [&name, this](const Symbol& s) { 
            return s.name == name && s.scopeDepth <= currentScope; 
        });
}

int SymbolTable::allocateGlobal() {
    return nextGlobalIndex++;
}

int SymbolTable::allocateLocal() {
    return currentStackOffset++;
}

void SymbolTable::resetStackOffset() {
    currentStackOffset = 0;
}

void SymbolTable::printSymbols() const {
    std::cout << "Symbol Table (scope=" << currentScope << "):\n";
    for (const auto& symbol : symbols) {
        std::cout << "  " << symbol.name 
                  << " (scope=" << symbol.scopeDepth
                  << ", type=" << (symbol.symbolType == SymbolType::VARIABLE ? "var" :
                                  symbol.symbolType == SymbolType::FUNCTION ? "func" : "param")
                  << ", datatype=" << (symbol.dataType == DataType::INT ? "int" : "void")
                  << ", global=" << symbol.isGlobal;
        if (symbol.isGlobal) {
            std::cout << ", globalIndex=" << symbol.globalIndex;
        } else {
            std::cout << ", stackOffset=" << symbol.stackOffset;
        }
        std::cout << ")\n";
    }
}

void SymbolTable::initializeBuiltins() {
    // Arduino built-in functions
    declareSymbol("pinMode", SymbolType::FUNCTION, DataType::VOID);
    declareSymbol("digitalWrite", SymbolType::FUNCTION, DataType::VOID);
    declareSymbol("digitalRead", SymbolType::FUNCTION, DataType::INT);
    declareSymbol("analogWrite", SymbolType::FUNCTION, DataType::VOID);
    declareSymbol("analogRead", SymbolType::FUNCTION, DataType::INT);
    declareSymbol("delay", SymbolType::FUNCTION, DataType::VOID);
    declareSymbol("millis", SymbolType::FUNCTION, DataType::INT);
    declareSymbol("micros", SymbolType::FUNCTION, DataType::INT);
    declareSymbol("printf", SymbolType::FUNCTION, DataType::VOID);
}