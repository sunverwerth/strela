#include "NameResolver.h"
#include "Ast/nodes.h"
#include "Types/types.h"
#include "exceptions.h"
#include "Scope.h"

namespace Strela {
    NameResolver::NameResolver(Scope* globals): scope(globals) {
    }

    void NameResolver::visit(AstModDecl& n) {
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

        visitChildren(n.functions);
        visitChildren(n.classes);

        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(AstClassDecl& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        for (auto&& field: n.fields) {
            scope->add(field->name, field);
        }
        for (auto&& method: n.methods) {
            method->params.insert(method->params.begin(), new AstParam(method->startToken, "this", new AstIdTypeExpr(method->startToken, n.name)));
            scope->add(method->name, method);
        }

        visitChildren(n.fields);
        visitChildren(n.methods);

        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(AstFuncDecl& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        if (n.returnTypeExpr) visitChild(n.returnTypeExpr);
        visitChildren(n.params);
        visitChildren(n.stmts);

        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(AstParam& n) {
        visitChild(n.typeExpr);
        scope->add(n.name, &n);
    }

    void NameResolver::visit(AstVarDecl& n) {
        if (n.typeExpr) {
            visitChild(n.typeExpr);
        }
        if (n.initializer) {
            visitChild(n.initializer);
        }
        scope->add(n.name, &n);
    }

    void NameResolver::visit(AstIdExpr& n) {
        n.symbol = scope->find(n.name);
        if (!n.symbol) {
            throw UnresolvedSymbolException(n.name, n);
        }
    }

    void NameResolver::visit(AstRetStmt& n) {
        if (n.expression) {
            visitChild(n.expression);
        }
    }

    void NameResolver::visit(AstExprStmt& n) {
        visitChild(n.expression);
    }

    void NameResolver::visit(AstLitExpr&) {
    }

    void NameResolver::visit(AstCallExpr& n) {
        visitChild(n.callTarget);
        visitChildren(n.arguments);
    }

    void NameResolver::visit(AstBlockStmt& n) {
        auto oldscope = scope;
        scope = new Scope(scope);

        visitChildren(n.stmts);
        
        delete scope;
        scope = oldscope;
    }

    void NameResolver::visit(AstBinopExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void NameResolver::visit(AstScopeExpr& n) {
        visitChild(n.scopeTarget);
    }

    void NameResolver::visit(AstIfStmt& n) {
        visitChild(n.condition);
        visitChild(n.trueBranch);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
    }

    void NameResolver::visit(AstFieldDecl& n) {
        visitChild(n.typeExpr);
    }

    void NameResolver::visit(AstNewExpr& n) {
        visitChild(n.typeExpr);
    }

    void NameResolver::visit(AstAssignExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void NameResolver::visit(AstIdTypeExpr& n) {
        n.symbol = scope->find(n.name);
        if (!n.symbol) {
            throw UnresolvedSymbolException(n.name, n);
        }
    }

    void NameResolver::visit(AstWhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void NameResolver::visit(AstPostfixExpr& n) {
        visitChild(n.target);
    }

    void NameResolver::visit(AstArrayTypeExpr& n) {
        visitChild(n.base);
    }

    void NameResolver::visit(AstImportStmt& n) {
    }

    void NameResolver::visit(AstUnaryExpr& n) {
        visitChild(n.target);
    }
}