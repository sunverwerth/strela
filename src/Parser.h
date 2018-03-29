// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Parser_h
#define Strela_Parser_h

#include "Ast/nodes.h"
#include "Ast/Token.h"
#include "SourceFile.h"
#include "Pass.h"

#include <vector>
#include <memory>

namespace Strela {
    class Parser: public Pass {
    public:
        Parser(const SourceFile& source, int starttoken = 0): source(source), tokens(source.tokens), token(tokens.begin() + starttoken) {}

        ModDecl* parseModDecl();
        IdExpr* parseIdExpr();
        FuncDecl* parseFuncDecl(bool external = false);
        Param* parseParam();
        VarDecl* parseVarDecl();
        BlockStmt* parseBlockStmt();
        ClassDecl* parseClassDecl();
        FieldDecl* parseFieldDecl();
        EnumDecl* parseEnumDecl();
        ImportStmt* parseImportStmt();
        InterfaceDecl* parseInterfaceDecl();
        InterfaceMethodDecl* parseInterfaceMethodDecl();

        Stmt* parseStmt();
        RetStmt* parseRetStmt();
        ExprStmt* parseExprStmt();
        IfStmt* parseIfStmt();
        WhileStmt* parseWhileStmt();

        Expr* parseExpr(int precedence = 0);
        NewExpr* parseNewExpr();
        LitExpr* parseLitExpr();
        CallExpr* parseCallExpr(Expr* callTarget);
        ArrayLitExpr* parseArrayLitExpr();
        SubscriptExpr* parseSubscriptExpr(Expr* callTarget);

        TypeExpr* parseTypeExpr();
        GenericReificationExpr* parseGenericReificationExpr(TypeExpr* target);

        bool match(TokenType type);
        bool matchExpr();
        bool matchSecondary();
        bool matchUnary();
        bool matchTypeSecondary();
        bool matchBinary();
        void expect(TokenType type);
        Token eat(TokenType type);
        bool eatOptional(TokenType type);
        Token eat();
        bool eof();
        void expected(TokenType expectedType);
        void expected(const std::string& expected);

    private:
        const std::vector<Token>& tokens;
        const SourceFile& source;
        std::vector<Token>::const_iterator token;

        int numVariables = 0;

        template <typename T> T* addPosition(T* node, const Token& tok) {
            node->line = tok.line;
            node->column = tok.column;
            node->source = &source;
            node->firstToken = tok.index;
            return node;
        }

    };
}

#endif