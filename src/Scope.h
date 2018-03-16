#ifndef Strela_Scope_h
#define Strela_Scope_h

#include <map>

namespace Strela {
    class Type;
    class Node;

    enum class SymbolKind {
        Type,
        Node,
    };

    class Symbol {
    public:
        Symbol(Type* type): kind(SymbolKind::Type), type(type) {}
        Symbol(Node* node): kind(SymbolKind::Node), node(node) {}
        
    public:
        SymbolKind kind;
        union {
            Type* type;
            Node* node;
        };
		// for overloaded functions
		Symbol* next = nullptr;
    };

    class Scope {
    public:
        Scope(Scope* parent): parent(parent) {}
        ~Scope();
        
        Scope* getParent();
        void setParent(Scope* p);
        void add(const std::string& name, Type* type);
        void add(const std::string& name, Node* node);
        Symbol* find(const std::string& name);
    
    private:
        void add(const std::string& name, Symbol* symbol);
        
    private:
        Scope* parent;
        std::map<std::string, Symbol*> symbols;
    };
}

#endif