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

    ModDecl* Parser::parseModule() {
        auto startToken = eat(TokenType::Module);
        auto name = eat(TokenType::Identifier).value;
        while (match(TokenType::Period)) {
            name += eat().value;
            name += eat(TokenType::Identifier).value;
        }

        eat(TokenType::CurlyOpen);

        std::vector<FuncDecl*> functions;
        std::vector<ClassDecl*> classes;
        std::vector<ImportStmt*> imports;
        std::vector<EnumDecl*> enums;

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

        return addPosition(new ModDecl(name, imports, functions, classes, enums), startToken);
    }

    ImportStmt* Parser::parseImportStmt() {
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
        return addPosition(new ImportStmt(parts, all), st);
    }

    FuncDecl* Parser::parseFunction() {
        numVariables = 0;
        auto startToken = eat(TokenType::Function);
        auto name = eat(TokenType::Identifier);
        eat(TokenType::ParenOpen);
        std::vector<Param*> parameters;
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
        TypeExpr* returnTypeExpr = nullptr;
        if (match(TokenType::Colon)) {
            eat();
            returnTypeExpr = parseTypeExpr();
        }
        else {
            returnTypeExpr = new IdTypeExpr("void");
        }
        
        eat(TokenType::CurlyOpen);
        std::vector<Stmt*> stmts;
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Semicolon)) {
                eat();
            }
            else {
                stmts.push_back(parseStatement());
            }
        }
        eat(TokenType::CurlyClose);
        auto fun = new FuncDecl(name.value, parameters, returnTypeExpr, stmts);
        fun->numVariables = numVariables;
        return addPosition(fun, startToken);
    }

    BlockStmt* Parser::parseBlockStatement() {
        auto startToken = eat(TokenType::CurlyOpen);
        std::vector<Stmt*> stmts;
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Semicolon)) {
                eat();
            }
            else {
                stmts.push_back(parseStatement());
            }
        }
        eat(TokenType::CurlyClose);
        return addPosition(new BlockStmt(stmts), startToken);
    }

    Stmt* Parser::parseStatement() {
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

    ExprStmt* Parser::parseExprStmt() {
        auto startToken = *token;
        auto expr = new ExprStmt(parseExpression());
        eat(TokenType::Semicolon);
        return addPosition(expr, startToken);
    }

    RetStmt* Parser::parseReturnStatement() {
        auto startToken = eat(TokenType::Return);
        Expr* expr = nullptr;
        if (matchExpr()) {
            expr = parseExpression();
        }
        auto ret = new RetStmt(expr);
        eat(TokenType::Semicolon);
        return addPosition(ret, startToken);
    }

    IfStmt* Parser::parseIfStmt() {
        auto startToken = eat(TokenType::If);
        eat(TokenType::ParenOpen);
        auto condition = parseExpression();
        eat(TokenType::ParenClose);
        auto trueBranch = parseStatement();
        Stmt* falseBranch = nullptr;
        if (match(TokenType::Else)) {
            eat();
            falseBranch = parseStatement();
        }
        return addPosition(new IfStmt(condition, trueBranch, falseBranch), startToken);
    }

    WhileStmt* Parser::parseWhileStmt() {
        auto startToken = eat(TokenType::While);
        eat(TokenType::ParenOpen);
        auto condition = parseExpression();
        eat(TokenType::ParenClose);
        auto body = parseStatement();
        return addPosition(new WhileStmt(condition, body), startToken);
    }

    TypeExpr* Parser::parseTypeExpr() {
        TypeExpr* expression = nullptr;
        if (match(TokenType::Identifier)) {
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
            else {
                expected("secondary type expression.");
            }
        }

        return expression;
    }
    
    Expr* Parser::parseExpression(int precedence) {
        Expr* expression = nullptr;
        if (match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null)) {
            expression = parseLiteralExpression();
        }
        else if (match(TokenType::Identifier)) {
            expression = parseIdentifierExpression();
        }
        else if (matchUnary()) {
            auto st = eat();
            expression = addPosition(new UnaryExpr(st.type, parseExpression(unaryPrecedenceMap[st.type])), st);
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
                error(*token, "No precedence for operator '" + token->value + "'");
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
                expression = addPosition(new ScopeExpr(expression, name), st);
            }
            else if (match(TokenType::MinusMinus) || match(TokenType::PlusPlus)) {
                auto st = eat();
                expression = addPosition(new PostfixExpr(expression, st.type), st);
            }
            else if (matchBinary()) {
				auto op = eat();
                if (op.type == TokenType::Equals) {
                    expression = addPosition(new AssignExpr(expression, parseExpression(myprec)), op);
                }
                else {
                    expression = addPosition(new BinopExpr(op.type, expression, parseExpression(myprec)), op);
                }
            }
            else {
                expected("secondary expression");
                break;
            }
        }
        return expression;
    }

    NewExpr* Parser::parseNewExpr() {
        auto startToken = eat(TokenType::New);
        auto typeExpr = parseTypeExpr();
        return addPosition(new NewExpr(typeExpr), startToken);
    }

    CallExpr* Parser::parseCallExpr(Expr* callTarget) {
        auto startToken = eat(TokenType::ParenOpen);
        std::vector<Expr*> arguments;
        while (!eof() && !match(TokenType::ParenClose)) {
            arguments.push_back(parseExpression());
            if (match(TokenType::Comma)) {
                eat();
            }
        }
        eat(TokenType::ParenClose);
        return addPosition(new CallExpr(callTarget, arguments), startToken);
    }

    LitExpr* Parser::parseLiteralExpression() {
        if (!(match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null))) {
            expected("literal");
        }
        auto tok = eat();
        return addPosition(new LitExpr(tok), tok);
    }

    IdExpr* Parser::parseIdentifierExpression() {
        auto startToken = eat(TokenType::Identifier);
        return addPosition(new IdExpr(startToken.value), startToken);
    }

    Param* Parser::parseParameter() {
        auto startToken = eat(TokenType::Identifier);
        eat(TokenType::Colon);
        auto type = parseTypeExpr();
        return addPosition(new Param(startToken.value, type), startToken);
    }

    VarDecl* Parser::parseVarDecl() {
        auto startToken = eat(TokenType::Var);
        auto name = eat(TokenType::Identifier).value;
        TypeExpr* type = nullptr;
        if (match(TokenType::Colon)) {
            eat();
            type = parseTypeExpr();
        }
        Expr* initializer = nullptr;
        if (match(TokenType::Equals)) {
            eat();
            initializer = parseExpression();
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

    ClassDecl* Parser::parseClassDecl() {
        auto startToken = eat(TokenType::Class);
        auto name = eat(TokenType::Identifier).value;
        eat(TokenType::CurlyOpen);
        std::vector<FuncDecl*> methods;
        std::vector<FieldDecl*> fields;
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
        return addPosition(new ClassDecl(name, methods, fields), startToken);
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

            if (match(TokenType::Comma)) {
                eat();
            }
            else {
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
        std::string msg = "Unexpected '" + getTokenName(token->type) + "', expected '" + getTokenName(expectedType) + "'.";
        throw UnexpectedTokenException(source.filename + ":" + std::to_string(token->line) + ":" + std::to_string(token->column) + " Error: " + msg);
    }

    void Parser::expected(const std::string& expected) {
        std::string msg = "Unexpected '" + getTokenName(token->type) + "', expected " + expected + ".";
        throw UnexpectedTokenException(source.filename + ":" + std::to_string(token->line) + ":" + std::to_string(token->column) + " Error: " + msg);
    }

    void Parser::error(const Token& t, const std::string& msg) {
        throw ParseException(source.filename + ":" + std::to_string(t.line) + ":" + std::to_string(t.column) + " Error: " + msg);
    }

}