// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Scope_h
#define Strela_Scope_h

#include <map>

namespace Strela {
    class Node;

    class Symbol {
    public:
        Symbol(Node* node): node(node) {}
        
    public:
        Node* node;
		// for overloaded functions
		Symbol* next = nullptr;
    };

    class Scope {
    public:
        Scope(Scope* parent): parent(parent) {}
        ~Scope();
        
        Scope* getParent();
        void setParent(Scope* p);
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