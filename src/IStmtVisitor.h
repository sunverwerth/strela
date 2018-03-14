#ifndef Strela_IStmtVisitor_h
#define Strela_IStmtVisitor_h

namespace Strela {
    class AstModDecl;
    class AstFuncDecl;
    class AstParam;
    class AstVarDecl;
    class AstRetStmt;
    class AstBlockStmt;
    class AstExprStmt;
    class AstClassDecl;
    class AstIfStmt;
    class AstFieldDecl;
    class AstWhileStmt;
    class AstImportStmt;
    class AstEnumDecl;
    class AstEnumElement;

    class IStmtVisitor {
    public:
        virtual void visit(AstModDecl&) = 0;
        virtual void visit(AstImportStmt&) = 0;
        virtual void visit(AstFuncDecl&) = 0;
        virtual void visit(AstParam&) = 0;
        virtual void visit(AstVarDecl&) = 0;
        virtual void visit(AstClassDecl&) = 0;
        virtual void visit(AstFieldDecl&) = 0;
        virtual void visit(AstEnumDecl&) = 0;
        virtual void visit(AstEnumElement&) = 0;

        virtual void visit(AstRetStmt&) = 0;
        virtual void visit(AstBlockStmt&) = 0;
        virtual void visit(AstExprStmt&) = 0;
        virtual void visit(AstIfStmt&) = 0;
        virtual void visit(AstWhileStmt&) = 0;
    };
}

#endif