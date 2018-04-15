// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

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
        {TokenType::Is, 0},

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

        // cast
        {TokenType::Colon, 13},

        // access
        {TokenType::Period, 14},
        {TokenType::BracketOpen, 14},
        {TokenType::ParenOpen, 14},
        {TokenType::PlusPlus, 14},
        {TokenType::MinusMinus, 14},
    };

    ModDecl* Parser::parseModDecl() {
        auto startToken = eat(TokenType::Module);
        auto name = eat(TokenType::Identifier).value;
        while (eatOptional(TokenType::Period)) {
            name += ".";
            name += eat(TokenType::Identifier).value;
        }

        eat(TokenType::CurlyOpen);

        std::vector<FuncDecl*> functions;
        std::vector<ClassDecl*> classes;
        std::vector<InterfaceDecl*> interfaces;
        std::vector<ImportStmt*> imports;
        std::vector<EnumDecl*> enums;

        bool exportNext = false;
        bool externalNext = false;

        while (!eof() && !match(TokenType::CurlyClose)) {
            if (eatOptional(TokenType::Export)) {
                exportNext = true;
                continue;
            }

            if (eatOptional(TokenType::External)) {
                externalNext = true;
                continue;
            }

            if (match(TokenType::Function)) {
                auto fun = parseFuncDecl(externalNext);
                if (exportNext) fun->isExported = true;
                functions.push_back(fun);
            }
            else if (match(TokenType::Class)) {
                auto cls = parseClassDecl();
                if (exportNext) cls->isExported = true;
                classes.push_back(cls);
            }
            else if (match(TokenType::Interface)) {
                auto iface = parseInterfaceDecl();
                if (exportNext) iface->isExported = true;
                interfaces.push_back(iface);
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
                eat();
                expected("module member declaration");
            }

            exportNext = false;
            externalNext = false;
        }
        eat(TokenType::CurlyClose);

        return addPosition(new ModDecl(name, imports, functions, classes, interfaces, enums), startToken);
    }

    ImportStmt* Parser::parseImportStmt() {
        auto st = eat(TokenType::Import);
        std::vector<std::string> parts;
        bool all = false;
        parts.push_back(eat(TokenType::Identifier).value);
        while (eatOptional(TokenType::Period)) {
            if (eatOptional(TokenType::Asterisk)) {
                all = true;
                break;
            }
            else {
                parts.push_back(eat(TokenType::Identifier).value);
            }
        }
        eat(TokenType::Semicolon);
        return addPosition(new ImportStmt(parts, all), st);
    }

    FuncDecl* Parser::parseFuncDecl(bool external) {
        numVariables = 0;
        auto startToken = eat(TokenType::Function);
        std::string name;
        if (match(TokenType::Identifier)) {
            name = eat().value;
        }
        else if (match(TokenType::BracketOpen)) {
            eat();
            eat(TokenType::BracketClose);
            name = "[]";
        }
        else {
            expected("function name");
        }
        eat(TokenType::ParenOpen);
        std::vector<Param*> parameters;
        while (!eof() && !match(TokenType::ParenClose)) {
            auto param = parseParam();
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
        TypeExpr* returnTypeExpr = nullptr;
        
        if (eatOptional(TokenType::Colon)) {
            returnTypeExpr = parseTypeExpr();
        }
        else {
            returnTypeExpr = new IdTypeExpr("void");
        }

        std::vector<Stmt*> stmts;
        if (external) {
            eat(TokenType::Semicolon);
        }
        else {        
            eat(TokenType::CurlyOpen);
            while (!eof() && !match(TokenType::CurlyClose)) {
                if (match(TokenType::Semicolon)) {
                    eat();
                }
                else {
                    stmts.push_back(parseStmt());
                }
            }
            eat(TokenType::CurlyClose);
        }

        auto fun = new FuncDecl(name, parameters, returnTypeExpr, stmts);
        fun->numVariables = numVariables;
        fun->isExternal = external;
        return addPosition(fun, startToken);
    }

    BlockStmt* Parser::parseBlockStmt() {
        auto startToken = eat(TokenType::CurlyOpen);
        std::vector<Stmt*> stmts;
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Semicolon)) {
                eat();
            }
            else {
                stmts.push_back(parseStmt());
            }
        }
        eat(TokenType::CurlyClose);
        return addPosition(new BlockStmt(stmts), startToken);
    }

    Stmt* Parser::parseStmt() {
        if (match(TokenType::Return)) {
            return parseRetStmt();
        }
        else if (match(TokenType::CurlyOpen)) {
            return parseBlockStmt();
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
            eat();
            expected("statement");
            return nullptr; // TODO: return null object
        }
    }

    ExprStmt* Parser::parseExprStmt() {
        auto startToken = *token;
        auto expr = new ExprStmt(parseExpr());
        eat(TokenType::Semicolon);
        return addPosition(expr, startToken);
    }

    RetStmt* Parser::parseRetStmt() {
        auto startToken = eat(TokenType::Return);
        Expr* expr = nullptr;
        if (matchExpr()) {
            expr = parseExpr();
        }
        auto ret = new RetStmt(expr);
        eat(TokenType::Semicolon);
        return addPosition(ret, startToken);
    }

    IfStmt* Parser::parseIfStmt() {
        auto startToken = eat(TokenType::If);
        eat(TokenType::ParenOpen);
        auto condition = parseExpr();
        eat(TokenType::ParenClose);
        auto trueBranch = parseStmt();
        Stmt* falseBranch = nullptr;
        if (eatOptional(TokenType::Else)) {
            falseBranch = parseStmt();
        }
        return addPosition(new IfStmt(condition, trueBranch, falseBranch), startToken);
    }

    WhileStmt* Parser::parseWhileStmt() {
        auto startToken = eat(TokenType::While);
        eat(TokenType::ParenOpen);
        auto condition = parseExpr();
        eat(TokenType::ParenClose);
        auto body = parseStmt();
        return addPosition(new WhileStmt(condition, body), startToken);
    }

    TypeExpr* Parser::parseTypeExpr() {
        TypeExpr* expression = nullptr;
        if (match(TokenType::Identifier) || match(TokenType::Null)) {
            auto start = eat();
            expression = addPosition(new IdTypeExpr(start.value), start);
        }
        else {
            expected(TokenType::Identifier);
        }

        while (matchTypeSecondary()) {
            if (match(TokenType::BracketOpen)) {
                auto st = eat();
                eat(TokenType::BracketClose);
                expression = addPosition(new ArrayTypeExpr(expression), st);
            }
            else if (match(TokenType::LessThan)) {
                expression = parseGenericReificationExpr(expression);
            }
            else if (match(TokenType::Pipe)) {
                auto st = eat();
                expression = addPosition(new UnionTypeExpr(expression, parseTypeExpr()), st);
            }
            else if (match(TokenType::QuestionMark)) {
                auto st = eat();
                expression = addPosition(new NullableTypeExpr(expression), st);
            }
            else {
                eat();
                expected("secondary type expression.");
            }
        }

        return expression;
    }

    GenericReificationExpr* Parser::parseGenericReificationExpr(TypeExpr* target) {
        auto startToken = eat(TokenType::LessThan);
        std::vector<TypeExpr*> genericArguments;
        while (!eof() && !match(TokenType::GreaterThan)) {
            genericArguments.push_back(parseTypeExpr());
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::GreaterThan);
        return addPosition(new GenericReificationExpr(target, genericArguments), startToken);
    }
    
    Expr* Parser::parseExpr(int precedence) {
        Expr* expression = nullptr;
        if (match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null)) {
            expression = parseLitExpr();
        }
        else if (match(TokenType::Identifier)) {
            expression = parseIdExpr();
        }
        else if (matchUnary()) {
            auto st = eat();
            expression = addPosition(new UnaryExpr(st.type, parseExpr(unaryPrecedenceMap[st.type])), st);
        }
        else if (match(TokenType::ParenOpen)) {
            eat();
            expression = parseExpr();
            eat(TokenType::ParenClose);
        }
        else if (match(TokenType::New)) {
            expression = parseNewExpr();
        }
        else if (match(TokenType::This)) {
            auto st = eat();
            expression = addPosition(new ThisExpr(), st);
        }
        else if (match(TokenType::BracketOpen)) {
            expression = parseArrayLitExpr();
        }
        else {
            expression = new LitExpr(eat());
            expected("primary expression or prefix operator");
        }

        while (matchSecondary()) {
            auto it = precedenceMap.find(token->type);
            if (it == precedenceMap.end()) {
                error(source, *token, "No precedence for operator '" + token->value + "'");
                eat();
                continue;
            }
            if (it->second < precedence) {
                break;
            }
            int myprec = it->second;

            if (match(TokenType::ParenOpen)) {
                expression = parseCallExpr(expression);
            }
            else if (match(TokenType::BracketOpen)) {
                expression = parseSubscriptExpr(expression);
            }
            else if (match(TokenType::Period)) {
                auto st = eat();
                auto name = eat(TokenType::Identifier).value;
                expression = addPosition(new ScopeExpr(expression, name), st);
            }
            else if (match(TokenType::MinusMinus) || match(TokenType::PlusPlus)) {
                auto st = eat();
                expression = addPosition(new PostfixExpr(expression, st.type), st);
            }
            else if (match(TokenType::Is)) {
                auto startToken = eat(TokenType::Is);
                expression = addPosition(new IsExpr(expression, parseTypeExpr()), startToken);
            }
            else if (match(TokenType::Colon)) {
                auto startToken = eat(TokenType::Colon);
                expression = addPosition(new CastExpr(expression, parseTypeExpr()), startToken);
            }
            else if (matchBinary()) {
				auto op = eat();
                if (
                    op.type == TokenType::Equals ||
                    op.type == TokenType::PlusEquals ||
                    op.type == TokenType::MinusEquals ||
                    op.type == TokenType::AsteriskEquals ||
                    op.type == TokenType::SlashEquals
                ) {
                    expression = addPosition(new AssignExpr(op.type, expression, parseExpr(myprec)), op);
                }
                else {
                    expression = addPosition(new BinopExpr(op.type, expression, parseExpr(myprec)), op);
                }
            }
            else {
                eat();
                expected("secondary expression");
                break;
            }
        }
        return expression;
    }

    NewExpr* Parser::parseNewExpr() {
        auto startToken = eat(TokenType::New);
        auto typeExpr = parseTypeExpr();
        
        std::vector<Expr*> arguments;
        if (eatOptional(TokenType::ParenOpen)) {
            while (!eof() && !match(TokenType::ParenClose)) {
                arguments.push_back(parseExpr());
                if (!eatOptional(TokenType::Comma)) {
                    break;
                }
            }
            eat(TokenType::ParenClose);
        }
        return addPosition(new NewExpr(typeExpr, arguments), startToken);
    }

    CallExpr* Parser::parseCallExpr(Expr* callTarget) {
        auto startToken = eat(TokenType::ParenOpen);
        std::vector<Expr*> arguments;
        while (!eof() && !match(TokenType::ParenClose)) {
            arguments.push_back(parseExpr());
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::ParenClose);
        return addPosition(new CallExpr(callTarget, arguments), startToken);
    }

    SubscriptExpr* Parser::parseSubscriptExpr(Expr* callTarget) {
        auto startToken = eat(TokenType::BracketOpen);
        std::vector<Expr*> arguments;
        while (!eof() && !match(TokenType::BracketClose)) {
            arguments.push_back(parseExpr());
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::BracketClose);
        return addPosition(new SubscriptExpr(callTarget, arguments), startToken);
    }

    LitExpr* Parser::parseLitExpr() {
        if (!(match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null))) {
            expected("literal");
        }
        auto tok = eat();
        return addPosition(new LitExpr(tok), tok);
    }

    IdExpr* Parser::parseIdExpr() {
        auto startToken = eat(TokenType::Identifier);
        return addPosition(new IdExpr(startToken.value), startToken);
    }

    Param* Parser::parseParam() {
        auto startToken = eat(TokenType::Identifier);
        eat(TokenType::Colon);
        auto type = parseTypeExpr();
        return addPosition(new Param(startToken.value, type), startToken);
    }

    VarDecl* Parser::parseVarDecl() {
        auto startToken = eat(TokenType::Var);
        auto name = eat(TokenType::Identifier).value;
        TypeExpr* type = nullptr;

        if (eatOptional(TokenType::Colon)) {
            type = parseTypeExpr();
        }

        Expr* initializer = nullptr;

        if (eatOptional(TokenType::Equals)) {
            initializer = parseExpr();
        }

        if (!type && !initializer) {
            expected("variable initializer.");
        }

        eat(TokenType::Semicolon);
        auto vardecl = addPosition(new VarDecl(name, type, initializer), startToken);
        vardecl->index = numVariables++;
        return vardecl;
    }

    FieldDecl* Parser::parseFieldDecl() {
        auto startToken = eat(TokenType::Var);
        auto name = eat(TokenType::Identifier).value;
        eat(TokenType::Colon);
        auto type = parseTypeExpr();
        eat(TokenType::Semicolon);
        return addPosition(new FieldDecl(name, type), startToken);
    }

    ArrayLitExpr* Parser::parseArrayLitExpr() {
        auto startToken = eat(TokenType::BracketOpen);
        std::vector<Expr*> elements;
        while (!eof() && !match(TokenType::BracketClose)) {
            elements.push_back(parseExpr());
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::BracketClose);
        return addPosition(new ArrayLitExpr(elements), startToken);
    }

    ClassDecl* Parser::parseClassDecl() {
        auto startToken = eat(TokenType::Class);
        auto name = eat(TokenType::Identifier).value;

        std::vector<GenericParam*> genericParams;
        if (eatOptional(TokenType::LessThan)) {
            while (!eof() && !match(TokenType::GreaterThan)) {
                auto tok = eat(TokenType::Identifier);
                genericParams.push_back(addPosition(new GenericParam(tok.value), tok));
                if (match(TokenType::Comma)) {
                    eat();
                }
            }
            eat(TokenType::GreaterThan);
        }

        std::vector<FuncDecl*> methods;
        std::vector<FieldDecl*> fields;
        eat(TokenType::CurlyOpen);
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Function)) {
                methods.push_back(parseFuncDecl());
            }
            else if (match(TokenType::Var)) {
                auto field = parseFieldDecl();
                field->index = fields.size();
                fields.push_back(field);
            }
            else {
                eat();
                expected("class member declaration");
            }
        }
        eat(TokenType::CurlyClose);
        return addPosition(new ClassDecl(name, genericParams, methods, fields), startToken);
    }

    InterfaceDecl* Parser::parseInterfaceDecl() {
        auto startToken = eat(TokenType::Interface);
        auto name = eat(TokenType::Identifier).value;
        std::vector<InterfaceMethodDecl*> methods;
        eat(TokenType::CurlyOpen);
        while (!eof() && !match(TokenType::CurlyClose)) {
            auto im = parseInterfaceMethodDecl();
            im->index = methods.size();
            methods.push_back(im);
            eat(TokenType::Semicolon);
        }
        eat(TokenType::CurlyClose);
        return addPosition(new InterfaceDecl(name, methods), startToken);
    }

    InterfaceMethodDecl* Parser::parseInterfaceMethodDecl() {
        auto startToken = eat(TokenType::Function);
        auto name = eat(TokenType::Identifier).value;
        eat(TokenType::ParenOpen);
        std::vector<Param*> parameters;
        while (!eof() && !match(TokenType::ParenClose)) {
            auto param = parseParam();
            param->index = parameters.size();
            parameters.push_back(param);
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::ParenClose);
        eat(TokenType::Colon);
        auto returnTypeExpr = parseTypeExpr();
        
        return addPosition(new InterfaceMethodDecl(name, parameters, returnTypeExpr), startToken);
    }

    EnumDecl* Parser::parseEnumDecl() {
        auto startToken = eat(TokenType::Enum);
        auto name = eat(TokenType::Identifier).value;
        eat(TokenType::CurlyOpen);
        std::vector<EnumElement*> elements;
        while (match(TokenType::Identifier)) {
            auto tok = eat(TokenType::Identifier);
            auto el = addPosition(new EnumElement(tok.value), tok);
            el->index = elements.size();
            elements.push_back(el);

            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::CurlyClose);
        return addPosition(new EnumDecl(name, elements), startToken);
    }

    bool Parser::match(TokenType type) {
        return !eof() && token->type == type;
    }

    bool Parser::matchExpr() {
        return
            match(TokenType::BracketOpen) ||
            match(TokenType::This) ||
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
            match(TokenType::Is) ||
            match(TokenType::BracketOpen) ||
            match(TokenType::Colon) ||
            matchBinary();
    }

    bool Parser::matchBinary() {
        return
            match(TokenType::Plus) ||
            match(TokenType::Minus) ||
            match(TokenType::Asterisk) ||
            match(TokenType::Slash) ||
            match(TokenType::Equals) ||
            match(TokenType::PlusEquals) ||
            match(TokenType::MinusEquals) ||
            match(TokenType::AsteriskEquals) ||
            match(TokenType::SlashEquals) ||
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
            match(TokenType::BracketOpen) ||
            match(TokenType::LessThan) ||
            match(TokenType::Pipe) ||
            match(TokenType::QuestionMark)
            ;
    }

    void Parser::expect(TokenType type) {
        if (!match(type)) {
            expected(type);
        }
    }

    Token Parser::eat(TokenType type) {
        expect(type);
        return eat();
    }
    
    bool Parser::eatOptional(TokenType type) {
        if (match(type)) {
            eat();
            return true;
        }
        return false;
    }
    
    Token Parser::eat() {
        if (eof()) return *token;
        else return *token++;
    }

    bool Parser::eof() {
        return token == tokens.end();
    }

    void Parser::expected(TokenType expectedType) {
        error(source, *token, "Unexpected '" + getTokenName(token->type) + "', expected " + getTokenName(expectedType) + ".");
    }

    void Parser::expected(const std::string& expected) {
        error(source, *token, "Unexpected '" + getTokenName(token->type) + "', expected " + expected + ".");
    }
}