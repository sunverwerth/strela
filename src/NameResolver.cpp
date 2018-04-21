// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "NameResolver.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Scope.h"
#include "SourceFile.h"

namespace Strela {
    NameResolver::NameResolver(Scope* globals): scope(globals) {
    }

    int NameResolver::resolveGenerics(ModDecl& n) {
        int numGenerics = 0;

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

                auto en = imp->module->getEnum(imp->parts.back());
                if (en) {
                    if (en->isExported) {
                        scope->add(en->name, en);
                        continue;
                    }
                    else {
                        throw Exception(en->name + " is not exported.");
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

        for (auto& cls: n.classes) {
            for (auto& gen: cls->reifiedClasses) {
                if (!gen->isResolved) {
                    gen->accept(*this);
                    numGenerics++;
                }
            }
        }

        delete scope;
        scope = oldscope;
        
        return numGenerics;
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

                auto en = imp->module->getEnum(imp->parts.back());
                if (en) {
                    if (en->isExported) {
                        scope->add(en->name, en);
                        continue;
                    }
                    else {
                        throw Exception(en->name + " is not exported.");
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
        // is generic
        if (n.genericParams.size() > 0 && n.genericArguments.empty()) return;

        // is reified generic
        if (n.genericParams.size() > 0 && n.genericArguments.size() > 0) {
            if (n.isResolved) return;
            n.isResolved = true;
        }

        auto oldscope = scope;
        scope = new Scope(scope);

        auto oldclass = _class;
        _class = &n;

        for (int i = 0; i < n.genericParams.size(); ++i) {
            scope->add(n.genericParams[i]->name, n.genericArguments[i]);
        }

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

        n._class = _class;
        n._interface = _interface;

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
        n.type = UnionType::get(n.base->type, n.next->type);
    }

    void NameResolver::visit(IdExpr& n) {
        auto symbol = scope->find(n.name);
        if (!symbol) {
            error(n, "Unresolved symbol '" + n.name + "'.");
            return;
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
        else if (auto field = symbol->node->as<FieldDecl>()) {
            n.node = field;
            n.type = field->type;
            auto _this = new ThisExpr();
            _this->_class = _class;
            _this->type = _class;
            n.context = _this;
        }
        else if (auto fun = symbol->node->as<FuncDecl>()) {
            if (fun->_class) {
                auto _this = new ThisExpr();
                _this->_class = _class;
                _this->type = _class;
                n.context = _this;
            }
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

    void NameResolver::visit(FieldDecl& n) {
        visitChild(n.typeExpr);
        n.type = n.typeExpr->type;
    }

    void NameResolver::visit(NewExpr& n) {
        visitChild(n.typeExpr);
        visitChildren(n.arguments);
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
            error(n, "'" + n.name + "' does not name a type.");
        }
    }

    void NameResolver::visit(ScopeTypeExpr& n) {
        visitChild(n.target);
        auto member = n.target->type->getMember(n.name);
        if (auto type = member->as<TypeDecl>()) {
            n.type = type;
        }
        else {
            error(n, n.name + " is not a type.");
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
    
    void NameResolver::visit(ArrayLitExpr& n) {
        visitChildren(n.elements);
    }

    void NameResolver::visit(SubscriptExpr& n) {
        visitChildren(n.arguments);
        visitChild(n.callTarget);
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

    void NameResolver::visit(NullableTypeExpr& n) {
        visitChild(n.base);
        n.type = UnionType::get(n.base->type, &NullType::instance);
    }

    void NameResolver::visit(GenericParam&) {
    }

    void NameResolver::visit(GenericReificationExpr& n) {
        visitChild(n.base);
        visitChildren(n.genericArguments);

        if (auto cls = n.base->type->as<ClassDecl>()) {
            if (cls->genericParams.size() > 0) {
                if (cls->genericParams.size() == n.genericArguments.size()) {
                    std::vector<TypeDecl*> types;
                    for (auto&& tex: n.genericArguments) {
                        types.push_back(tex->type);
                    }
                    n.type = cls->getReifiedClass(types);
                }
                else {
                    error(n, "Expected " + std::to_string(cls->genericParams.size()) + " type arguments, " + std::to_string(n.genericArguments.size()) + " given.");
                }
            }
            else {
                error(n, "Can not reify non-generic type '" + n.base->type->name + "'.");
            }
        }
        else {
            error(n, "Can not reify non-generic type '" + n.base->type->name + "'.");
        }
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
}