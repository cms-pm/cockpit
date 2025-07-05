#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <unordered_map>

enum class SymbolType {
    VARIABLE,
    FUNCTION,
    PARAMETER
};

enum class DataType {
    INT,
    VOID
};

struct Symbol {
    std::string name;
    SymbolType symbolType;
    DataType dataType;
    int scopeDepth;
    int stackOffset;      // For local variables
    int globalIndex;      // For global variables
    bool isGlobal;
    
    Symbol(const std::string& n, SymbolType st, DataType dt, int scope)
        : name(n), symbolType(st), dataType(dt), scopeDepth(scope)
        , stackOffset(-1), globalIndex(-1), isGlobal(scope == 0) {}
};

class SymbolTable {
private:
    std::vector<Symbol> symbols;
    int currentScope;
    int nextGlobalIndex;
    int currentStackOffset;
    
public:
    SymbolTable();
    
    // Scope management
    void enterScope();
    void exitScope();
    int getCurrentScope() const { return currentScope; }
    
    // Symbol operations
    bool declareSymbol(const std::string& name, SymbolType type, DataType dataType);
    Symbol* lookupSymbol(const std::string& name);
    bool isSymbolDeclared(const std::string& name) const;
    
    // Memory allocation
    int allocateGlobal();
    int allocateLocal();
    void resetStackOffset();
    
    // Debug and inspection
    void printSymbols() const;
    size_t getSymbolCount() const { return symbols.size(); }
    
    // Arduino built-in functions
    void initializeBuiltins();
};

#endif // SYMBOL_TABLE_H