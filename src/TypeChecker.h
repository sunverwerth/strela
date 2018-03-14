#ifndef Strela_TypeChecker_h
#define Strela_TypeChecker_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"


#include <string>
#include <vector>

namespace Strela {
    class Type;
    class AstNode;
    class AstStmt;
    class AstExpr;

    class TypeChecker: public IStmtVisitor, public IExprVisitor {
    public:
        TypeChecker();

        void visit(AstModDecl&) override;
        void visit(AstFuncDecl&) override;
        void visit(AstParam&) override;
        void visit(AstVarDecl&) override;
        void visit(AstIdExpr&) override;
        void visit(AstRetStmt&) override;
        void visit(AstLitExpr&) override;
        void visit(AstBlockStmt&) override;
        void visit(AstCallExpr&) override;
        void visit(AstExprStmt&) override;
        void visit(AstBinopExpr&) override;
        void visit(AstClassDecl&) override;
        void visit(AstScopeExpr&) override;
        void visit(AstIfStmt&) override;
        void visit(AstFieldDecl&) override;
        void visit(AstNewExpr&) override;
        void visit(AstAssignExpr&) override;
        void visit(AstIdTypeExpr&) override;
        void visit(AstWhileStmt&) override;
        void visit(AstPostfixExpr&) override;
        void visit(AstArrayTypeExpr&) override;
        void visit(AstImportStmt&) override;
        void visit(AstUnaryExpr&) override;
        void visit(AstEnumDecl&) override;
        void visit(AstEnumElement&) override;

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            child->accept(*this);
        }

    private:
		AstFuncDecl* findOverload(AstExpr* target, const std::vector<Type*>& argtypes);
        void error(AstNode& node, const std::string& msg);
        void warning(AstNode& node, const std::string& msg);

        AstFuncDecl* function = nullptr;
        AstBlockStmt* block = nullptr;
        AstClassDecl* _class = nullptr;

        bool returns = false;
    };
}
#endif