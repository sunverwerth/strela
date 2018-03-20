#include "NameResolver.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Scope.h"
#include "SourceFile.h"

namespace Strela {
    NameResolver::NameResolver(Scope* globals): scope(globals) {
    }

    void NameResolver::visit(ModDecl& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        scope->add(n.name, &n);

        for (auto&& imp: n.imports) {
            if (imp->all) {
                for (auto&& fun: imp->module->functions) {
                    if (fun->isExported) {
                        scope->add(fun->name, fun);
                    }
                }
                for (auto&& cls: imp->module->classes) {
                    if (cls->isExported) {
                        scope->add(cls->name, cls);
                    }
                }
                for (auto&& iface: imp->module->interfaces) {
                    if (iface->isExported) {
                        scope->add(iface->name, iface);
                    }
                }
                for (auto&& en: imp->module->enums) {
                    if (en->isExported) {
                        scope->add(en->name, en);
                    }
                }
            }
            else if (imp->importModule) {
                scope->add(imp->parts.back(), imp->module);
            }
            else {
                auto cls = imp->module->getClass(imp->parts.back());
                if (cls) {
                    if (cls->isExported) {
                        scope->add(cls->name, cls);
                        continue;
                    }
                    else {
                        throw Exception(cls->name + " is not exported.");
                    }
                }

                auto funs = imp->module->getFunctions(imp->parts.back());
                if (!funs.empty()) {
                    for (auto&& fun: funs) {
                        if (fun->isExported) {
                            scope->add(fun->name, fun);
                        }
                    }
                }
            }
        }

        for (auto&& fun: n.functions) {
            scope->add(fun->name, fun);
        }
        for (auto&& cls: n.classes) {
            scope->add(cls->name, cls);
        }
        for (auto&& iface: n.interfaces) {
            scope->add(iface->name, iface);
        }
        for (auto&& en: n.enums) {
            scope->add(en->name, en);
        }

        visitChildren(n.functions);
        visitChildren(n.classes);
        visitChildren(n.interfaces);

        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(ClassDecl& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        auto oldclass = _class;
        _class = &n;

        for (auto&& field: n.fields) {
            scope->add(field->name, field);
        }
        for (auto&& method: n.methods) {
            scope->add(method->name, method);
        }

        visitChildren(n.fields);
        visitChildren(n.methods);

        _class = oldclass;

        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(FuncDecl& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        visitChild(n.returnTypeExpr);
        visitChildren(n.params);
        visitChildren(n.stmts);

        std::vector<TypeDecl*> paramTypes;
        for (auto&& param: n.params) {
            paramTypes.push_back(param->typeExpr->type);
        }
        n.type = FuncType::get(n.returnTypeExpr->type, paramTypes);

        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(Param& n) {
        visitChild(n.typeExpr);
        n.type = n.typeExpr->type;
        scope->add(n.name, &n);
    }

    void NameResolver::visit(VarDecl& n) {
        if (n.typeExpr) {
            visitChild(n.typeExpr);
            n.type = n.typeExpr->type;
        }
        else {
            n.type = &InvalidType::instance;
        }
        if (n.initializer) {
            visitChild(n.initializer);
        }
        scope->add(n.name, &n);
    }

    void NameResolver::visit(IdExpr& n) {
        auto symbol = scope->find(n.name);
        if (!symbol) {
            error(n, "Unresolved symbol '" + n.name + "'.");
        }

        if (auto td = symbol->node->as<TypeDecl>()) {
            n.type = &TypeType::instance;
            n.node = td;
        }
        else if (auto var = symbol->node->as<VarDecl>()) {
            n.node = var;
            n.type = var->type;
        }
        else if (auto par = symbol->node->as<Param>()) {
            n.node = par;
            n.type = par->typeExpr->type;
        }
        else if (auto fun = symbol->node->as<FuncDecl>()) {
            if (symbol->next) {
                auto cur = symbol;
                while (cur) {
                    n.candidates.push_back(cur->node->as<FuncDecl>());
                    cur = cur->next;
                }
                n.type = &OverloadedFuncType::instance;
                n.node = nullptr;
            }
            else {
                n.node = fun;
                n.type = fun->type;
            }
        }
        else {
            error(n, "Unhandled symbol kind.");
            n.type = &InvalidType::instance;
        }
    }

    void NameResolver::visit(RetStmt& n) {
        if (n.expression) {
            visitChild(n.expression);
        }
    }

    void NameResolver::visit(ExprStmt& n) {
        visitChild(n.expression);
    }

    void NameResolver::visit(LitExpr&) {
    }

    void NameResolver::visit(CallExpr& n) {
        visitChild(n.callTarget);
        visitChildren(n.arguments);
    }

    void NameResolver::visit(BlockStmt& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        visitChildren(n.stmts);
        
        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(BinopExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void NameResolver::visit(ScopeExpr& n) {
        visitChild(n.scopeTarget);
    }

    void NameResolver::visit(IfStmt& n) {
        visitChild(n.condition);
        visitChild(n.trueBranch);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
    }

    void NameResolver::visit(FieldDecl& n) {
        visitChild(n.typeExpr);
        n.type = n.typeExpr->type;
    }

    void NameResolver::visit(NewExpr& n) {
        visitChild(n.typeExpr);
    }

    void NameResolver::visit(AssignExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void NameResolver::visit(IdTypeExpr& n) {
        auto symbol = scope->find(n.name);
        if (!symbol) {
            error(n, "Unresolved symbol '" + n.name + "'.");
        }
        
        if (auto td = symbol->node->as<TypeDecl>()) {
            n.type = td;
        }
        else {
            n.type = &InvalidType::instance;
            error(n, "'" + n.name + "' does not name a type.");
        }
    }

    void NameResolver::visit(WhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void NameResolver::visit(PostfixExpr& n) {
        visitChild(n.target);
    }

    void NameResolver::visit(ArrayTypeExpr& n) {
        visitChild(n.base);
        n.type = ArrayType::get(n.base->type);
    }

    void NameResolver::visit(ImportStmt& n) {
    }

    void NameResolver::visit(UnaryExpr& n) {
        visitChild(n.target);
    }

    void NameResolver::visit(EnumDecl& n) {
        visitChildren(n.elements);
    }

    void NameResolver::visit(EnumElement& n) {
    }

    void NameResolver::visit(InterfaceDecl& n) {
        auto oldinterface = _interface;
        _interface = &n;
        visitChildren(n.methods);
        _interface = oldinterface;
    }

    void NameResolver::visit(InterfaceMethodDecl& n) {
        visitChild(n.returnTypeExpr);
        visitChildren(n.params);

        std::vector<TypeDecl*> paramTypes;
        for (auto&& param: n.params) {
            paramTypes.push_back(param->typeExpr->type);
        }
        n.type = FuncType::get(n.returnTypeExpr->type, paramTypes);
    }

    void NameResolver::error(const Node& n, const std::string& msg) {
        if (n.source) {
            throw Exception(n.source->filename + ":" + std::to_string(n.line) + ":" + std::to_string(n.column) + " Error: " + msg);
        }
        else {
            throw Exception("Error: " + msg);
        }
    }
}