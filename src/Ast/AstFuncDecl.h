#ifndef Strela_Ast_AstFuncDecl_h
#define Strela_Ast_AstFuncDecl_h

#include "AstStmt.h"
#include "../Types/types.h"

#include <string>
#include <vector>

namespace Strela {
    class AstBlockStmt;
    class AstParam;
    class AstExpr;
    class AstTypeExpr;
    class FunctionType;
    class Type;
    class ByteCodeChunk;

    class AstFuncDecl: public AstStmt {
    public:
        AstFuncDecl(const Token& startToken, const std::string& name, const std::vector<AstParam*>& params, AstTypeExpr* returnTypeExpr, const std::vector<AstStmt*>& stmts): AstStmt(startToken), name(name), params(params), returnTypeExpr(returnTypeExpr), stmts(stmts), declType(Types::invalid), returnType(Types::invalid) {}
        STRELA_GET_TYPE(Strela::AstFuncDecl, Strela::AstStmt);
        STRELA_IMPL_VISITOR;

    public:
        bool isExported = false;
        std::string name;
        std::vector<AstParam*> params;
        AstTypeExpr* returnTypeExpr;
        std::vector<AstStmt*> stmts;
        Type* declType;
        Type* returnType;
        size_t opcodeStart = 0xdeadbeef;
        int numVariables = 0;
        AstClassDecl* parent = nullptr;
    };
}

#endif