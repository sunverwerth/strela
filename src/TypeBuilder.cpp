#include "TypeBuilder.h"
#include "Ast/nodes.h"
#include "Types/types.h"
#include "exceptions.h"
#include "Scope.h"

namespace Strela {
    TypeBuilder::TypeBuilder() {
    }

    void TypeBuilder::visit(ModDecl& n) {
        visitChildren(n.functions);
        visitChildren(n.classes);
    }

    void TypeBuilder::visit(ClassDecl& n) {
        visitChildren(n.fields);
        visitChildren(n.methods);
    }

    void TypeBuilder::visit(FuncDecl& n) {
        if (n.returnTypeExpr) visitChild(n.returnTypeExpr);
        visitChildren(n.params);
        visitChildren(n.stmts);
        n.returnType = n.returnTypeExpr ? n.returnTypeExpr->staticValue.type : Types::_void;


        n.declType = FunctionType::get(&n);
    }

    void TypeBuilder::visit(Param& n) {
        visitChild(n.typeExpr);
        n.declType = n.typeExpr->staticValue.type;
    }

    void TypeBuilder::visit(VarDecl& n) {
        n.declType = Types::invalid;
        if (n.typeExpr) {
            visitChild(n.typeExpr);
            n.declType = n.typeExpr->staticValue.type;
        }
        if (n.initializer) {
            visitChild(n.initializer);
        }
    }

    void TypeBuilder::visit(IdExpr& n) {
    }

    void TypeBuilder::visit(RetStmt& n) {
        if (n.expression) {
            visitChild(n.expression);
        }
    }

    void TypeBuilder::visit(ExprStmt& n) {
        visitChild(n.expression);
    }

    void TypeBuilder::visit(LitExpr&) {
    }

    void TypeBuilder::visit(CallExpr& n) {
        visitChild(n.callTarget);
        visitChildren(n.arguments);
    }

    void TypeBuilder::visit(BlockStmt& n) {
        visitChildren(n.stmts);
    }

    void TypeBuilder::visit(BinopExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void TypeBuilder::visit(ScopeExpr& n) {
        visitChild(n.scopeTarget);
    }

    void TypeBuilder::visit(IfStmt& n) {
        visitChild(n.condition);
        visitChild(n.trueBranch);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
    }

    void TypeBuilder::visit(FieldDecl& n) {
        visitChild(n.typeExpr);
        n.declType = n.typeExpr->staticValue.type;
    }

    void TypeBuilder::visit(NewExpr& n) {
        visitChild(n.typeExpr);
    }

    void TypeBuilder::visit(AssignExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void TypeBuilder::visit(IdTypeExpr& n) {
        if (n.symbol->kind == SymbolKind::Type) {
            n.type = Types::typetype;
            n.isStatic = true;
            n.staticValue.type = n.symbol->type;
        }
        else {
            if (auto cls = n.symbol->node->as<ClassDecl>()) {
                n.type = Types::typetype;
                n.isStatic = true;
                n.staticValue.type = cls->declType;
            }
            else if (auto mod = n.symbol->node->as<ModDecl>()) {
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

    void TypeBuilder::visit(WhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void TypeBuilder::visit(PostfixExpr& n) {
        visitChild(n.target);
    }

    void TypeBuilder::visit(ArrayTypeExpr& n) {
        visitChild(n.base);
        n.type = Types::typetype;
        n.isStatic = true;
        n.staticValue.type = ArrayType::get(n.base->staticValue.type);
    }

    void TypeBuilder::visit(ImportStmt& n) {
    }

    void TypeBuilder::visit(UnaryExpr& n) {
        visitChild(n.target);
    }

    void TypeBuilder::visit(EnumDecl& n) {
        visitChildren(n.elements);
    }

    void TypeBuilder::visit(EnumElement& n) {
    }

}