#include "Scope.h"
#include "exceptions.h"
#include "Ast/AstFuncDecl.h"

namespace Strela {
    Scope* Scope::getParent() {
        return parent;
    }

    Scope::~Scope() {
        for (auto&& it: symbols) {
           //delete it.second;
        }
    }

    void Scope::add(const std::string& name, Type* type) {
        add(name, new Symbol(type));
    }

    void Scope::add(const std::string& name, AstNode* node) {
        add(name, new Symbol(node));
    }

    void Scope::add(const std::string& name, Symbol* symbol) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
			if (symbol->kind == SymbolKind::Node && symbol->node->as<AstFuncDecl>() && it->second->kind == SymbolKind::Node && it->second->node->as<AstFuncDecl>()) {
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