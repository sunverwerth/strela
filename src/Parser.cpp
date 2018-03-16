#include "Parser.h"
#include "Ast/nodes.h"
#include "exceptions.h"

#include <memory>

namespace Strela {
    std::map<TokenType, int> unaryPrecedenceMap {
        {TokenType::PlusPlus, 13},
        {TokenType::MinusMinus, 13},

        {TokenType::Minus, 12},
        {TokenType::Tilde, 12},
        {TokenType::ExclamationMark, 12},
    };

    std::map<TokenType, int> precedenceMap {
        // Assignment
        {TokenType::Equals, 1},
        {TokenType::PlusEquals, 1},
        {TokenType::MinusEquals, 1},
        {TokenType::AsteriskEquals, 1},
        {TokenType::SlashEquals, 1},
        {TokenType::PercentEquals, 1},
        {TokenType::AmpEquals, 1},
        {TokenType::PipeEquals, 1},
        {TokenType::CaretEquals, 1},

        // logical
        {TokenType::PipePipe, 2},
        {TokenType::AmpAmp, 3},

        // binary
        {TokenType::Pipe, 4},
        {TokenType::Caret, 5},
        {TokenType::Amp, 6},

        // equality
        {TokenType::EqualsEquals, 7},
        {TokenType::ExclamationMarkEquals, 7},

        // comparison
        {TokenType::LessThan, 8},
        {TokenType::GreaterThan, 8},
        {TokenType::LessThanEquals, 8},
        {TokenType::GreaterThanEquals, 8},

        // bit shift

        // addition
        {TokenType::Plus, 10},
        {TokenType::Minus, 10},

        // multiplication
        {TokenType::Asterisk, 11},
        {TokenType::Slash, 11},
        {TokenType::Percent, 11},

        {TokenType::ExclamationMark, 12},
        {TokenType::Tilde, 12},

        // access
        {TokenType::Period, 13},
        {TokenType::BracketOpen, 13},
        {TokenType::ParenOpen, 13},
        {TokenType::PlusPlus, 13},
        {TokenType::MinusMinus, 13},
    };

    AstModDecl* Parser::parseModule() {
        auto startToken = eat(TokenType::Module);
        auto name = eat(TokenType::Identifier).value;
        while (match(TokenType::Period)) {
            name += eat().value;
            name += eat(TokenType::Identifier).value;
        }

        eat(TokenType::CurlyOpen);

        std::vector<AstFuncDecl*> functions;
        std::vector<AstClassDecl*> classes;
        std::vector<AstImportStmt*> imports;
        std::vector<AstEnumDecl*> enums;

        while (!eof() && !match(TokenType::CurlyClose)) {
            bool exportNext = match(TokenType::Export);
            if (exportNext) eat();

            if (match(TokenType::Function)) {
                auto fun = parseFunction();
                if (exportNext) fun->isExported = true;
                functions.push_back(fun);
            }
            else if (match(TokenType::Class)) {
                auto cls = parseClassDecl();
                if (exportNext) cls->isExported = true;
                classes.push_back(cls);
            }
            else if (match(TokenType::Enum)) {
                auto en = parseEnumDecl();
                if (exportNext) en->isExported = true;
                enums.push_back(en);
            }
            else if (match(TokenType::Import)) {
                imports.push_back(parseImportStmt());
            }
            else {
                expected("module member declaration");
            }
        }
        eat(TokenType::CurlyClose);

        return new AstModDecl(startToken, name, imports, functions, classes, enums);
    }

    AstImportStmt* Parser::parseImportStmt() {
        auto st = eat(TokenType::Import);
        std::vector<std::string> parts;
        bool all = false;
        parts.push_back(eat(TokenType::Identifier).value);
        while (match(TokenType::Period)) {
            eat();
            if (match(TokenType::Asterisk)) {
                eat();
                all = true;
                break;
            }
            else {
                parts.push_back(eat(TokenType::Identifier).value);
            }
        }
        eat(TokenType::Semicolon);
        return new AstImportStmt(st, parts, all);
    }

    AstFuncDecl* Parser::parseFunction() {
        numVariables = 0;
        auto startToken = eat(TokenType::Function);
        auto name = eat(TokenType::Identifier);
        eat(TokenType::ParenOpen);
        std::vector<AstParam*> parameters;
        while (!eof() && !match(TokenType::ParenClose)) {
            auto param = parseParameter();
            param->index = parameters.size();
            parameters.push_back(param);
            if (match(TokenType::Comma)) {
                eat();
            }
            else {
                break;
            }
        }
        eat(TokenType::ParenClose);
        AstTypeExpr* returnTypeExpr = nullptr;
        if (match(TokenType::Colon)) {
            eat();
            returnTypeExpr = parseTypeExpr();
        }
        
        eat(TokenType::CurlyOpen);
        std::vector<AstStmt*> stmts;
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Semicolon)) {
                eat();
            }
            else {
                stmts.push_back(parseStatement());
            }
        }
        eat(TokenType::CurlyClose);
        auto fun = new AstFuncDecl(startToken, name.value, parameters, returnTypeExpr, stmts);
        fun->numVariables = numVariables;
        return fun;
    }

    AstBlockStmt* Parser::parseBlockStatement() {
        auto startToken = eat(TokenType::CurlyOpen);
        std::vector<AstStmt*> stmts;
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Semicolon)) {
                eat();
            }
            else {
                stmts.push_back(parseStatement());
            }
        }
        eat(TokenType::CurlyClose);
        return new AstBlockStmt(startToken, stmts);
    }

    AstStmt* Parser::parseStatement() {
        if (match(TokenType::Return)) {
            return parseReturnStatement();
        }
        else if (match(TokenType::CurlyOpen)) {
            return parseBlockStatement();
        }
        else if (match(TokenType::If)) {
            return parseIfStmt();
        }
        else if (match(TokenType::While)) {
            return parseWhileStmt();
        }
        else if (match(TokenType::Var)) {
            return parseVarDecl();
        }
        else if (matchExpr()) {
            return parseExprStmt();
        }
        else {
            expected("statement");
        }
    }

    AstExprStmt* Parser::parseExprStmt() {
        auto startToken = *token;
        auto expr = new AstExprStmt(startToken, parseExpression());
        eat(TokenType::Semicolon);
        return expr;
    }

    AstRetStmt* Parser::parseReturnStatement() {
        auto startToken = eat(TokenType::Return);
        AstExpr* expr = nullptr;
        if (matchExpr()) {
            expr = parseExpression();
        }
        auto ret = new AstRetStmt(startToken, expr);
        eat(TokenType::Semicolon);
        return ret;
    }

    AstIfStmt* Parser::parseIfStmt() {
        auto startToken = eat(TokenType::If);
        eat(TokenType::ParenOpen);
        auto condition = parseExpression();
        eat(TokenType::ParenClose);
        auto trueBranch = parseStatement();
        AstStmt* falseBranch = nullptr;
        if (match(TokenType::Else)) {
            eat();
            falseBranch = parseStatement();
        }
        return new AstIfStmt(startToken, condition, trueBranch, falseBranch);
    }

    AstWhileStmt* Parser::parseWhileStmt() {
        auto startToken = eat(TokenType::While);
        eat(TokenType::ParenOpen);
        auto condition = parseExpression();
        eat(TokenType::ParenClose);
        auto body = parseStatement();
        return new AstWhileStmt(startToken, condition, body);
    }

    AstTypeExpr* Parser::parseTypeExpr() {
        AstTypeExpr* expression;
        if (match(TokenType::Identifier)) {
            auto start = eat();
            expression = new AstIdTypeExpr(start, start.value);
        }
        else {
            expected(TokenType::Identifier);
        }

        while (matchTypeSecondary()) {
            if (match(TokenType::BracketOpen)) {
                auto st = eat();
                eat(TokenType::BracketClose);
                expression = new AstArrayTypeExpr(st, expression);
            }
            else {
                expected("secondary type expression.");
            }
        }

        return expression;
    }
    
    AstExpr* Parser::parseExpression(int precedence) {
        AstExpr* expression = nullptr;
        if (match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null)) {
            expression = parseLiteralExpression();
        }
        else if (match(TokenType::Identifier)) {
            expression = parseIdentifierExpression();
        }
        else if (matchUnary()) {
            auto st = eat();
            expression = new AstUnaryExpr(st, parseExpression(unaryPrecedenceMap[st.type]));
        }
        else if (match(TokenType::ParenOpen)) {
            eat();
            expression = parseExpression();
            eat(TokenType::ParenClose);
        }
        else if (match(TokenType::New)) {
            expression = parseNewExpr();
        }
        else {
            expected("primary expression or prefix operator");
        }

        while (matchSecondary()) {
            auto it = precedenceMap.find(token->type);
            if (it == precedenceMap.end()) {
                throw ParseException(token->line, token->column, "No precedence for operator '" + token->value + "'");
            }
            if (it->second < precedence) {
                break;
            }
            int myprec = it->second;

            if (match(TokenType::ParenOpen)) {
                expression = parseCallExpr(expression);
            }
            else if (match(TokenType::Period)) {
                auto st = eat();
                auto name = eat(TokenType::Identifier).value;
                expression = new AstScopeExpr(st, expression, name);
            }
            else if (match(TokenType::MinusMinus) || match(TokenType::PlusPlus)) {
                auto st = eat();
                expression = new AstPostfixExpr(st, expression);
            }
            else if (matchBinary()) {
				auto op = eat();
                if (op.type == TokenType::Equals) {
                    expression = new AstAssignExpr(op, expression, parseExpression(myprec));
                }
                else {
                    expression = new AstBinopExpr(op, expression, parseExpression(myprec));
                }
            }
            else {
                expected("secondary expression");
                break;
            }
        }
        return expression;
    }

    AstNewExpr* Parser::parseNewExpr() {
        auto startToken = eat(TokenType::New);
        auto typeExpr = parseTypeExpr();
        return new AstNewExpr(startToken, typeExpr);
    }

    AstCallExpr* Parser::parseCallExpr(AstExpr* callTarget) {
        auto startToken = eat(TokenType::ParenOpen);
        std::vector<AstExpr*> arguments;
        while (!eof() && !match(TokenType::ParenClose)) {
            arguments.push_back(parseExpression());
            if (match(TokenType::Comma)) {
                eat();
            }
        }
        eat(TokenType::ParenClose);
        return new AstCallExpr(startToken, callTarget, arguments);
    }

    AstLitExpr* Parser::parseLiteralExpression() {
        if (!(match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null))) {
            expected("literal");
        }
        return new AstLitExpr(eat());
    }

    AstIdExpr* Parser::parseIdentifierExpression() {
        auto startToken = eat(TokenType::Identifier);
        return new AstIdExpr(startToken, startToken.value);
    }

    AstParam* Parser::parseParameter() {
        auto startToken = eat(TokenType::Identifier);
        eat(TokenType::Colon);
        auto type = parseTypeExpr();
        return new AstParam(startToken, startToken.value, type);
    }

    AstVarDecl* Parser::parseVarDecl() {
        auto startToken = eat(TokenType::Var);
        auto name = eat(TokenType::Identifier).value;
        AstTypeExpr* type = nullptr;
        if (match(TokenType::Colon)) {
            eat();
            type = parseTypeExpr();
        }
        AstExpr* initializer = nullptr;
        if (match(TokenType::Equals)) {
            eat();
            initializer = parseExpression();
        }
        if (!type && !initializer) {
            expected("variable initializer.");
        }
        eat(TokenType::Semicolon);
        auto vardecl = new AstVarDecl(startToken, name, type, initializer);
        vardecl->index = numVariables++;
        return vardecl;
    }

    AstFieldDecl* Parser::parseFieldDecl() {
        auto startToken = eat(TokenType::Var);
        auto name = eat(TokenType::Identifier).value;
        eat(TokenType::Colon);
        auto type = parseTypeExpr();
        eat(TokenType::Semicolon);
        return new AstFieldDecl(startToken, name, type);
    }

    AstClassDecl* Parser::parseClassDecl() {
        auto startToken = eat(TokenType::Class);
        auto name = eat(TokenType::Identifier).value;
        eat(TokenType::CurlyOpen);
        std::vector<AstFuncDecl*> methods;
        std::vector<AstFieldDecl*> fields;
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Function)) {
                methods.push_back(parseFunction());
            }
            else if (match(TokenType::Var)) {
                auto field = parseFieldDecl();
                field->index = fields.size();
                fields.push_back(field);
            }
            else {
                expected("class member declaration");
            }
        }
        eat(TokenType::CurlyClose);
        return new AstClassDecl(startToken, name, methods, fields);
    }

    AstEnumDecl* Parser::parseEnumDecl() {
        auto startToken = eat(TokenType::Enum);
        auto name = eat(TokenType::Identifier).value;
        eat(TokenType::CurlyOpen);
        std::vector<AstEnumElement*> elements;
        while (match(TokenType::Identifier)) {
            auto el = new AstEnumElement(eat(TokenType::Identifier));
            el->index = elements.size();
            elements.push_back(el);

            if (match(TokenType::Comma)) {
                eat();
            }
            else {
                break;
            }
        }
        eat(TokenType::CurlyClose);
        return new AstEnumDecl(startToken, name, elements);
    }

    bool Parser::match(TokenType type) {
        return !eof() && token->type == type;
    }

    bool Parser::matchExpr() {
        return
            match(TokenType::Null) ||
            match(TokenType::Integer) ||
            match(TokenType::Float) ||
            match(TokenType::String) ||
            match(TokenType::Boolean) ||
            match(TokenType::Identifier) ||
            match(TokenType::ParenOpen) ||
            match(TokenType::New) || 
            matchUnary()
            ;
    }

    bool Parser::matchUnary() {
        return
            match(TokenType::Minus) ||
            match(TokenType::ExclamationMark) ||
            match(TokenType::Tilde) ||
            match(TokenType::MinusMinus) ||
            match(TokenType::PlusPlus)
            ;
    }

    bool Parser::matchSecondary() {
        return
            match(TokenType::ParenOpen) ||
            match(TokenType::Period) ||
            match(TokenType::MinusMinus) ||
            match(TokenType::PlusPlus) ||
            matchBinary();
    }

    bool Parser::matchBinary() {
        return
            match(TokenType::Plus) ||
            match(TokenType::Minus) ||
            match(TokenType::Asterisk) ||
            match(TokenType::Slash) ||
            match(TokenType::Equals) ||
            match(TokenType::EqualsEquals) ||
            match(TokenType::ExclamationMarkEquals) ||
            match(TokenType::AmpAmp) ||
            match(TokenType::PipePipe) ||
            match(TokenType::LessThan) ||
            match(TokenType::GreaterThan) ||
            match(TokenType::LessThanEquals) ||
            match(TokenType::GreaterThanEquals)
            ;
    }

    bool Parser::matchTypeSecondary() {
        return
            match(TokenType::BracketOpen)
            ;
    }

    void Parser::expect(TokenType type) {
        if (!match(type)) {
            expected(type);
        }
    }

    Token Parser::eat(TokenType type) {
        expect(type);
        return *token++;
    }
    
    Token Parser::eat() {
        return *token++;
    }

    bool Parser::eof() {
        return token == tokens.end();
    }

    void Parser::expected(TokenType expectedType) {
        throw UnexpectedTokenException(*token, expectedType);
    }

    void Parser::expected(const std::string& expected) {
        throw UnexpectedTokenException(*token, expected);
    }

}