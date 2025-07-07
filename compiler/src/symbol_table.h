#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

enum class SymbolType {
    VARIABLE,
    FUNCTION,
    PARAMETER,
    ARRAY
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
    int globalIndex;      // For global variables/arrays
    bool isGlobal;
    
    // Array-specific fields
    size_t arraySize;     // For arrays only
    uint8_t arrayId;      // Array identifier for VM
    
    Symbol(const std::string& n, SymbolType st, DataType dt, int scope)
        : name(n), symbolType(st), dataType(dt), scopeDepth(scope)
        , stackOffset(-1), globalIndex(-1), isGlobal(scope == 0)
        , arraySize(0), arrayId(0) {}
};

class SymbolTable {
private:
    std::vector<Symbol> symbols;
    int currentScope;
    int nextGlobalIndex;
    int currentStackOffset;
    uint8_t nextArrayId;
    
public:
    SymbolTable();
    
    // Scope management
    void enterScope();
    void exitScope();
    int getCurrentScope() const { return currentScope; }
    
    // Symbol operations
    bool declareSymbol(const std::string& name, SymbolType type, DataType dataType);
    bool declareArray(const std::string& name, DataType dataType, size_t size);
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