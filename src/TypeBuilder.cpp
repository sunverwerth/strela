#include "TypeBuilder.h"
#include "Ast/nodes.h"
#include "Types/types.h"
#include "exceptions.h"
#include "Scope.h"

namespace Strela {
    TypeBuilder::TypeBuilder() {
    }

    void TypeBuilder::visit(AstModDecl& n) {
        visitChildren(n.functions);
        visitChildren(n.classes);
    }

    void TypeBuilder::visit(AstClassDecl& n) {
        visitChildren(n.fields);
        visitChildren(n.methods);
    }

    void TypeBuilder::visit(AstFuncDecl& n) {
        if (n.returnTypeExpr) visitChild(n.returnTypeExpr);
        visitChildren(n.params);
        visitChildren(n.stmts);
        n.returnType = n.returnTypeExpr ? n.returnTypeExpr->staticValue.type : Types::_void;


        n.declType = FunctionType::get(&n);
    }

    void TypeBuilder::visit(AstParam& n) {
        visitChild(n.typeExpr);
        n.declType = n.typeExpr->staticValue.type;
    }

    void TypeBuilder::visit(AstVarDecl& n) {
        n.declType = Types::invalid;
        if (n.typeExpr) {
            visitChild(n.typeExpr);
            n.declType = n.typeExpr->staticValue.type;
        }
        if (n.initializer) {
            visitChild(n.initializer);
        }
    }

    void TypeBuilder::visit(AstIdExpr& n) {
    }

    void TypeBuilder::visit(AstRetStmt& n) {
        if (n.expression) {
            visitChild(n.expression);
        }
    }

    void TypeBuilder::visit(AstExprStmt& n) {
        visitChild(n.expression);
    }

    void TypeBuilder::visit(AstLitExpr&) {
    }

    void TypeBuilder::visit(AstCallExpr& n) {
        visitChild(n.callTarget);
        visitChildren(n.arguments);
    }

    void TypeBuilder::visit(AstBlockStmt& n) {
        visitChildren(n.stmts);
    }

    void TypeBuilder::visit(AstBinopExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void TypeBuilder::visit(AstScopeExpr& n) {
        visitChild(n.scopeTarget);
    }

    void TypeBuilder::visit(AstIfStmt& n) {
        visitChild(n.condition);
        visitChild(n.trueBranch);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
    }

    void TypeBuilder::visit(AstFieldDecl& n) {
        visitChild(n.typeExpr);
        n.declType = n.typeExpr->staticValue.type;
    }

    void TypeBuilder::visit(AstNewExpr& n) {
        visitChild(n.typeExpr);
    }

    void TypeBuilder::visit(AstAssignExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void TypeBuilder::visit(AstIdTypeExpr& n) {
        if (n.symbol->kind == SymbolKind::Type) {
            n.type = Types::typetype;
            n.isStatic = true;
            n.staticValue.type = n.symbol->type;
        }
        else {
            if (auto cls = n.symbol->node->as<AstClassDecl>()) {
                n.type = Types::typetype;
                n.isStatic = true;
                n.staticValue.type = cls->declType;
            }
            else if (auto mod = n.symbol->node->as<AstModDecl>()) {
                n.type = Types::typetype;
                n.isStatic = true;
                n.staticValue.type = mod->declType;
            }
            else {
                throw Exception("'" + n.name + "' is not a type.");
                //error(n, "'" + n.name + "' is not a type.");
            }
        }
    }

    void TypeBuilder::visit(AstWhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void TypeBuilder::visit(AstPostfixExpr& n) {
        visitChild(n.target);
    }

    void TypeBuilder::visit(AstArrayTypeExpr& n) {
        visitChild(n.base);
        n.type = Types::typetype;
        n.isStatic = true;
        n.staticValue.type = ArrayType::get(n.base->staticValue.type);
    }

    void TypeBuilder::visit(AstImportStmt& n) {
    }

    void TypeBuilder::visit(AstUnaryExpr& n) {
        visitChild(n.target);
    }

    void TypeBuilder::visit(AstEnumDecl& n) {
        visitChildren(n.elements);
    }

    void TypeBuilder::visit(AstEnumElement& n) {
    }

}