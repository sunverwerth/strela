#ifndef Strela_Parser_h
#define Strela_Parser_h

#include "Ast/nodes.h"
#include "Ast/Token.h"

#include <vector>
#include <memory>

namespace Strela {
    class Parser {
    public:
        Parser(const std::vector<Token>& tokens): tokens(tokens), token(tokens.begin()) {}

        AstModDecl* parseModule();
        AstIdExpr* parseIdentifierExpression();
        AstFuncDecl* parseFunction();
        AstParam* parseParameter();
        AstVarDecl* parseVarDecl();
        AstBlockStmt* parseBlockStatement();
        AstClassDecl* parseClassDecl();
        AstFieldDecl* parseFieldDecl();
        AstImportStmt* parseImportStmt();

        AstStmt* parseStatement();
        AstRetStmt* parseReturnStatement();
        AstExprStmt* parseExprStmt();
        AstIfStmt* parseIfStmt();
        AstWhileStmt* parseWhileStmt();

        AstExpr* parseExpression(int precedence = 0);
        AstTypeExpr* parseTypeExpr();
        AstNewExpr* parseNewExpr();
        AstLitExpr* parseLiteralExpression();
        AstCallExpr* parseCallExpr(AstExpr* callTarget);

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

    private:
        const std::vector<Token>& tokens;
        std::vector<Token>::const_iterator token;

        int numVariables = 0;
    };
}

#endif