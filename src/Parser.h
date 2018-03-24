// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Parser_h
#define Strela_Parser_h

#include "Ast/nodes.h"
#include "Ast/Token.h"
#include "SourceFile.h"

#include <vector>
#include <memory>

namespace Strela {
    class Parser {
    public:
        Parser(const SourceFile& source, int starttoken = 0): source(source), tokens(source.tokens), token(&tokens[starttoken]) {}

        ModDecl* parseModule();
        IdExpr* parseIdentifierExpression();
        FuncDecl* parseFunction(bool external = false);
        Param* parseParameter();
        VarDecl* parseVarDecl();
        BlockStmt* parseBlockStatement();
        ClassDecl* parseClassDecl();
        FieldDecl* parseFieldDecl();
        EnumDecl* parseEnumDecl();
        ImportStmt* parseImportStmt();
        InterfaceDecl* parseInterfaceDecl();
        InterfaceMethodDecl* parseInterfaceMethodDecl();

        Stmt* parseStatement();
        RetStmt* parseReturnStatement();
        ExprStmt* parseExprStmt();
        IfStmt* parseIfStmt();
        WhileStmt* parseWhileStmt();

        Expr* parseExpression(int precedence = 0);
        TypeExpr* parseTypeExpr();
        NewExpr* parseNewExpr();
        LitExpr* parseLiteralExpression();
        CallExpr* parseCallExpr(Expr* callTarget);
        ArrayLitExpr* parseArrayLitExpr();
        SubscriptExpr* parseSubscriptExpr(Expr* callTarget);

        GenericReificationExpr* parseGenericReificationExpr(TypeExpr* target);

        bool match(TokenType type);
        bool matchExpr();
        bool matchSecondary();
        bool matchUnary();
        bool matchTypeSecondary();
        bool matchBinary();
        void expect(TokenType type);
        Token eat(TokenType type);
        Token eat();
        bool eof();
        void expected(TokenType expectedType);
        void expected(const std::string& expected);

        void error(const Token& t, const std::string& msg);

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