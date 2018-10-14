// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "NameResolver.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Scope.h"
#include "SourceFile.h"

namespace Strela {
    bool checkTypeExpr(Expr* e) {
        return e->type == &TypeType::instance && e->typeValue != &InvalidType::instance;
    }

    NameResolver::NameResolver(Scope* globals): scope(globals) {
    }

    int NameResolver::resolveGenerics(ModDecl& n) {
        int numGenerics = 0;

        auto oldscope = scope;
        scope = new Scope(scope);

        for (auto&& imp: n.imports) {
            if (imp->all) {
                for (auto&& fun: imp->module->functions) {
                    if (fun->isExported) {
                        scope->add(fun->name, fun);
                    }
                }
                for (auto&& cls: imp->module->classes) {
                    if (cls->isExported) {
                        scope->add(cls->_name, cls);
                    }
                }
                for (auto&& iface: imp->module->interfaces) {
                    if (iface->isExported) {
                        scope->add(iface->_name, iface);
                    }
                }
                for (auto&& en: imp->module->enums) {
                    if (en->isExported) {
                        scope->add(en->_name, en);
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
                        scope->add(cls->_name, cls);
                        continue;
                    }
                    else {
                        throw Exception(cls->_name + " is not exported.");
                    }
                }

                auto en = imp->module->getEnum(imp->parts.back());
                if (en) {
                    if (en->isExported) {
                        scope->add(en->_name, en);
                        continue;
                    }
                    else {
                        throw Exception(en->_name + " is not exported.");
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
            scope->add(cls->_name, cls);
        }
        for (auto&& iface: n.interfaces) {
            scope->add(iface->_name, iface);
        }
        for (auto&& en: n.enums) {
            scope->add(en->_name, en);
        }
        for (auto&& ta: n.typeAliases) {
            scope->add(ta->_name, ta);
        }

        for (auto& cls: n.classes) {
            for (auto& gen: cls->reifiedClasses) {
                if (!gen->isResolved) {
                    resolve(*gen);
                    numGenerics++;
                }
            }
        }

        delete scope;
        scope = oldscope;
        
        return numGenerics;
    }

    void NameResolver::resolve(ModDecl& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        for (auto& import: n.imports) {
            resolve(*import);
        }

        for (auto&& fun: n.functions) {
            scope->add(fun->name, fun);
        }
        for (auto&& cls: n.classes) {
            scope->add(cls->_name, cls);
        }
        for (auto&& iface: n.interfaces) {
            scope->add(iface->_name, iface);
        }
        for (auto&& en: n.enums) {
            scope->add(en->_name, en);
        }
        for (auto&& ta: n.typeAliases) {
            scope->add(ta->_name, ta);
        }

        for (auto&& fun: n.functions) {
            resolve(*fun);
        }
        for (auto&& iface: n.interfaces) {
            resolve(*iface);
        }
        for (auto&& cls: n.classes) {
            resolve(*cls);
        }
        for (auto&& ta: n.typeAliases) {
            resolve(*ta);
        }

        delete scope;
        scope = oldscope;
    }

    void NameResolver::resolve(ClassDecl& n) {
        // is generic
        if (n.genericParams.size() > 0 && n.genericArguments.empty()) return;

        // is reified generic
        if (n.genericParams.size() > 0 && n.genericArguments.size() > 0) {
            if (n.isResolved) return;
            n.isResolved = true;
        }

        auto oldscope = scope;
        scope = new Scope(scope);

        for (int i = 0; i < n.genericParams.size(); ++i) {
            scope->add(n.genericParams[i]->_name, n.genericArguments[i]);
        }

        for (auto&& field: n.fields) {
            resolve(*field);
        }
        for (auto&& method: n.methods) {
            resolve(*method);
        }

        delete scope;
        scope = oldscope;
    }

    void NameResolver::resolve(FuncDecl& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        if (n.returnTypeExpr) visitChild(n.returnTypeExpr);
        for (auto& param: n.params) {
            resolve(*param);
        }
        visitChildren(n.stmts);

        std::vector<TypeDecl*> paramTypes;
        for (auto&& param: n.params) {
            paramTypes.push_back(param->typeExpr->typeValue);
        }
        n.declType = FuncType::get(n.returnTypeExpr ? n.returnTypeExpr->typeValue : &VoidType::instance, paramTypes);

        delete scope;
        scope = oldscope;
    }

    void NameResolver::resolve(Param& n) {
        visitChild(n.typeExpr);
        n.declType = n.typeExpr->typeValue;
        scope->add(n.name, &n);
    }

    void NameResolver::visit(VarDecl& n) {
        if (n.typeExpr) {
            visitChild(n.typeExpr);
            n.declType = n.typeExpr->typeValue;
        }

        if (n.initializer) {
            visitChild(n.initializer);
        }
        scope->add(n.name, &n);
    }

    void NameResolver::visit(IsExpr& n) {
        visitChild(n.target);
        visitChild(n.typeExpr);
    }

    void NameResolver::visit(UnionTypeExpr& n) {
        visitChild(n.base);
        visitChild(n.next);
        n.type = &TypeType::instance;
        n.typeValue = UnionType::get(n.base->typeValue, n.next->typeValue);
    }

    void NameResolver::visit(IdExpr& n) {
        auto symbol = scope->find(n.name);
        if (!symbol) {
            error(n, "Unresolved symbol '" + n.name + "'.");
            return;
        }

        if (auto td = symbol->node->as<TypeDecl>()) {
            n.type = &TypeType::instance;
            n.typeValue = td;
        }
        else if (auto var = symbol->node->as<VarDecl>()) {
            n.node = var;
            n.type = var->declType;
        }
        else if (auto par = symbol->node->as<Param>()) {
            n.node = par;
            n.type = par->declType;
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
                n.type = fun->declType;
            }
        }
        else {
            error(n, "Unhandled symbol kind.");
            return;
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

    void NameResolver::visit(CastExpr& n) {
        visitChild(n.sourceExpr);
        visitChild(n.targetTypeExpr);
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

    void NameResolver::resolve(FieldDecl& n) {
        visitChild(n.typeExpr);
        n.declType = n.typeExpr->typeValue;
    }

    void NameResolver::visit(NewExpr& n) {
        visitChild(n.typeExpr);
        visitChildren(n.arguments);
    }

    void NameResolver::visit(AssignExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void NameResolver::visit(WhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void NameResolver::visit(PostfixExpr& n) {
        visitChild(n.target);
    }

    void NameResolver::visit(ArrayTypeExpr& n) {
        visitChild(n.baseTypeExpr);
        n.type = &TypeType::instance;
        if (n.baseTypeExpr->typeValue == &InvalidType::instance) {
            return;
        }
        n.typeValue = ArrayType::get(n.baseTypeExpr->typeValue);
    }

    void NameResolver::resolve(ImportStmt& n) {
        if (n.all) {
            for (auto&& fun: n.module->functions) {
                if (fun->isExported) {
                    scope->add(fun->name, fun);
                }
            }
            for (auto&& cls: n.module->classes) {
                if (cls->isExported) {
                    scope->add(cls->_name, cls);
                }
            }
            for (auto&& iface: n.module->interfaces) {
                if (iface->isExported) {
                    scope->add(iface->_name, iface);
                }
            }
            for (auto&& en: n.module->enums) {
                if (en->isExported) {
                    scope->add(en->_name, en);
                }
            }
            for (auto&& alias: n.module->typeAliases) {
                if (alias->isExported) {
                    scope->add(alias->_name, alias);
                }
            }
        }
        else if (n.importModule) {
            scope->add(n.parts.back(), n.module);
        }
        else {
            auto cls = n.module->getClass(n.parts.back());
            if (cls) {
                if (cls->isExported) {
                    scope->add(cls->_name, cls);
                    return;
                }
            }

            auto en = n.module->getEnum(n.parts.back());
            if (en) {
                if (en->isExported) {
                    scope->add(en->_name, en);
                    return;
                }
            }

            auto funs = n.module->getFunctions(n.parts.back());
            if (!funs.empty()) {
                for (auto&& fun: funs) {
                    if (fun->isExported) {
                        scope->add(fun->name, fun);
                    }
                }
                return;
            }

            auto alias = n.module->getAlias(n.parts.back());
            if (alias) {
                if (alias->isExported) {
                    scope->add(alias->_name, alias);
                    return;
                }
            }

            // nothing found
            error(n, "Symbol '" + n.parts.back() + "' not found in module '" + n.getBaseName() + "'");
        }
    }

    void NameResolver::visit(UnaryExpr& n) {
        visitChild(n.target);
    }
    
    void NameResolver::visit(ArrayLitExpr& n) {
        visitChildren(n.elements);
    }

    void NameResolver::visit(MapLitExpr& n) {
        visitChildren(n.keys);
        visitChildren(n.values);
    }

    void NameResolver::visit(SubscriptExpr& n) {
        visitChildren(n.arguments);
        visitChild(n.callTarget);
    }

    void NameResolver::resolve(InterfaceDecl& n) {
        for (auto& field: n.fields) {
            resolve(*field);
        }
        for (auto& method: n.methods) {
            resolve(*method);
        }
    }

    void NameResolver::visit(NullableTypeExpr& n) {
        visitChild(n.baseTypeExpr);
        n.type = &TypeType::instance;
        n.typeValue = UnionType::get(n.baseTypeExpr->typeValue, &NullType::instance);
    }

    void NameResolver::visit(GenericReificationExpr& n) {
        visitChild(n.baseTypeExpr);
        visitChildren(n.genericArguments);

        auto cls = n.baseTypeExpr->typeValue->as<ClassDecl>();
        if (!cls) {
            error(n, "Can not reify non-class type '" + n.baseTypeExpr->typeValue->getFullName() + "'.");
            return;
        }

        if (cls->genericParams.empty()) {
            error(n, "Can not reify non-generic class '" + n.baseTypeExpr->typeValue->getFullName() + "'.");
            return;
        }

        if (cls->genericParams.size() != n.genericArguments.size()) {
            error(n, "Expected " + std::to_string(cls->genericParams.size()) + " type arguments, " + std::to_string(n.genericArguments.size()) + " given.");
            return;
        }

        std::vector<TypeDecl*> types;
        for (auto&& tex: n.genericArguments) {
            types.push_back(tex->typeValue);
        }

        n.type = &TypeType::instance;
        n.typeValue = cls->getReifiedClass(types);
    }

    void NameResolver::resolve(InterfaceFieldDecl& n) {
        visitChild(n.typeExpr);
        n.declType = n.typeExpr->typeValue;
    }

    void NameResolver::resolve(InterfaceMethodDecl& n) {
        visitChild(n.returnTypeExpr);
        for (auto& param: n.params) {
            resolve(*param);
        }

        std::vector<TypeDecl*> paramTypes;
        for (auto&& param: n.params) {
            paramTypes.push_back(param->typeExpr->typeValue);
        }
        n.type = FuncType::get(n.returnTypeExpr ? n.returnTypeExpr->typeValue : &VoidType::instance, paramTypes);
    }

    void NameResolver::resolve(TypeAliasDecl& n) {
        visitChild(n.typeExpr);
    }
}