#include "Scope.h"
#include "exceptions.h"
#include "Ast/FuncDecl.h"

namespace Strela {
    Scope* Scope::getParent() {
        return parent;
    }

    Scope::~Scope() {
        for (auto&& it: symbols) {
           //delete it.second;
        }
    }

    void Scope::add(const std::string& name, Node* node) {
        add(name, new Symbol(node));
    }

    void Scope::add(const std::string& name, Symbol* symbol) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
			if (symbol->node->as<FuncDecl>() && it->second->node->as<FuncDecl>()) {
				symbol->next = it->second;
				it->second = symbol;
			}
			else {
				throw DuplicateSymbolException(name);
			}
        }
        
        symbols.insert(std::make_pair(name, symbol));
    }

    Symbol* Scope::find(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) return it->second;
        if (parent) return parent->find(name);
        return nullptr;
    }

    void Scope::setParent(Scope* p) {
        parent = p;
    }
}