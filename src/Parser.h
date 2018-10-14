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
        IdExpr* parseIdExpr(Node* parent);
        FuncDecl* parseFuncDecl(Node* parent, bool external = false);
        Param* parseParam(Node* parent);
        VarDecl* parseVarDecl(Node* parent);
        BlockStmt* parseBlockStmt(Node* parent);
        ClassDecl* parseClassDecl(Node* parent);
        FieldDecl* parseFieldDecl(Node* parent);
        EnumDecl* parseEnumDecl(Node* parent);
        ImportStmt* parseImportStmt(Node* parent);
        InterfaceDecl* parseInterfaceDecl(Node* parent);
        InterfaceMethodDecl* parseInterfaceMethodDecl(Node* parent);
        InterfaceFieldDecl* parseInterfaceFieldDecl(Node* parent);
        TypeAliasDecl* parseTypeAliasDecl(Node* parent);

        Stmt* parseStmt(Node* parent);
        RetStmt* parseRetStmt(Node* parent);
        ExprStmt* parseExprStmt(Node* parent);
        IfStmt* parseIfStmt(Node* parent);
        WhileStmt* parseWhileStmt(Node* parent);

        Expr* parseExpr(Node* parent, int precedence = 0);
        NewExpr* parseNewExpr(Node* parent);
        LitExpr* parseLitExpr(Node* parent);
        CallExpr* parseCallExpr(Node* parent, Expr* callTarget);
        ArrayLitExpr* parseArrayLitExpr(Node* parent);
        MapLitExpr* parseMapLitExpr(Node* parent);
        SubscriptExpr* parseSubscriptExpr(Node* parent, Expr* callTarget);

        Expr* parseTypeExpr(Node* parent);
        GenericReificationExpr* parseGenericReificationExpr(Node* parent, Expr* target);

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
			node->lineend = (token - 1)->line;
            node->column = tok.column;
            node->source = &source;
            node->firstToken = tok.index;
            return node;
        }

    };
}

#endif