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
        {TokenType::Is, 1},

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
        auto moddecl = new ModDecl();
        auto startToken = eat(TokenType::Module);
        moddecl->line = startToken.line;
        moddecl->column = startToken.column;

        auto name = eat(TokenType::Identifier).value;
        while (eatOptional(TokenType::Period)) {
            name += ".";
            name += eat(TokenType::Identifier).value;
        }
        moddecl->_name = name;

        eat(TokenType::CurlyOpen);

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
                auto fun = parseFuncDecl(moddecl, externalNext);
                if (exportNext) fun->isExported = true;
                moddecl->functions.push_back(fun);
            }
            else if (match(TokenType::Class)) {
                auto cls = parseClassDecl(moddecl);
                if (exportNext) cls->isExported = true;
                moddecl->classes.push_back(cls);
            }
            else if (match(TokenType::Interface)) {
                auto iface = parseInterfaceDecl(moddecl);
                if (exportNext) iface->isExported = true;
                moddecl->interfaces.push_back(iface);
            }
            else if (match(TokenType::Enum)) {
                auto en = parseEnumDecl(moddecl);
                if (exportNext) en->isExported = true;
                moddecl->enums.push_back(en);
            }
            else if (match(TokenType::Type)) {
                auto al = parseTypeAliasDecl(moddecl);
                moddecl->typeAliases.push_back(al);
            }
            else if (match(TokenType::Import)) {
                auto imp = parseImportStmt(moddecl);
                moddecl->imports.push_back(imp);
            }
            else {
                eat();
                expected("module member declaration");
            }

            exportNext = false;
            externalNext = false;
        }
        auto endToken = eat(TokenType::CurlyClose);
        moddecl->lineend = endToken.line;

        return moddecl;
    }

    ImportStmt* Parser::parseImportStmt(Node* parent) {
        auto import = new ImportStmt();
        import->parent = parent;
        auto st = eat(TokenType::Import);

        import->parts.push_back(eat(TokenType::Identifier).value);
        while (eatOptional(TokenType::Period)) {
            if (eatOptional(TokenType::Asterisk)) {
                import->all = true;
                break;
            }
            else {
                import->parts.push_back(eat(TokenType::Identifier).value);
            }
        }
        eat(TokenType::Semicolon);
        return addPosition(import, st);
    }

    FuncDecl* Parser::parseFuncDecl(Node* parent, bool external) {
        auto fun = new FuncDecl();
        fun->parent = parent;
        numVariables = 0;
        auto startToken = eat(TokenType::Function);
        if (match(TokenType::Identifier)) {
            fun->name = eat().value;
        }
        else if (match(TokenType::BracketOpen)) {
            eat();
            eat(TokenType::BracketClose);
            fun->name = "[]";
        }
        else if (matchBinary()) {
            fun->name = eat().value;
        }
        else {
            expected("function name");
        }

        eat(TokenType::ParenOpen);
        while (!eof() && !match(TokenType::ParenClose)) {
            auto param = parseParam(fun);
            param->index = fun->params.size();
            fun->params.push_back(param);
            if (match(TokenType::Comma)) {
                eat();
            }
            else {
                break;
            }
        }
        eat(TokenType::ParenClose);
        
        if (eatOptional(TokenType::Colon)) {
            fun->returnTypeExpr = parseTypeExpr(fun);
        }

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
                    fun->stmts.push_back(parseStmt(fun));
                }
            }
            eat(TokenType::CurlyClose);
        }

        fun->numVariables = numVariables;
        fun->isExternal = external;
        return addPosition(fun, startToken);
    }

    BlockStmt* Parser::parseBlockStmt(Node* parent) {
        auto block = new BlockStmt();
        block->parent = parent;
        auto startToken = eat(TokenType::CurlyOpen);

        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Semicolon)) {
                eat();
            }
            else {
                block->stmts.push_back(parseStmt(block));
            }
        }
        eat(TokenType::CurlyClose);
        return addPosition(block, startToken);
    }

    Stmt* Parser::parseStmt(Node* parent) {
        if (match(TokenType::Return)) {
            return parseRetStmt(parent);
        }
        else if (match(TokenType::CurlyOpen)) {
            return parseBlockStmt(parent);
        }
        else if (match(TokenType::If)) {
            return parseIfStmt(parent);
        }
        else if (match(TokenType::While)) {
            return parseWhileStmt(parent);
        }
        else if (match(TokenType::Var)) {
            return parseVarDecl(parent);
        }
        else if (matchExpr()) {
            return parseExprStmt(parent);
        }
        else {
            eat();
            expected("statement");
            return nullptr; // TODO: return null object
        }
    }

    ExprStmt* Parser::parseExprStmt(Node* parent) {
        auto stmt = new ExprStmt();
        stmt->parent = parent;
        auto startToken = *token;
        stmt->expression = parseExpr(stmt);
        eat(TokenType::Semicolon);
        return addPosition(stmt, startToken);
    }

    RetStmt* Parser::parseRetStmt(Node* parent) {
        auto ret = new RetStmt();
        ret->parent = parent;
        auto startToken = eat(TokenType::Return);
        if (matchExpr()) {
            ret->expression = parseExpr(ret);
        }
        eat(TokenType::Semicolon);
        return addPosition(ret, startToken);
    }

    IfStmt* Parser::parseIfStmt(Node* parent) {
        auto ifStmt = new IfStmt();
        ifStmt->parent = parent;

        auto startToken = eat(TokenType::If);
        eat(TokenType::ParenOpen);
        ifStmt->condition = parseExpr(ifStmt);
        eat(TokenType::ParenClose);
        ifStmt->trueBranch = parseStmt(ifStmt);
        if (eatOptional(TokenType::Else)) {
            ifStmt->falseBranch = parseStmt(ifStmt);
        }
        return addPosition(ifStmt, startToken);
    }

    WhileStmt* Parser::parseWhileStmt(Node* parent) {
        auto whileStmt = new WhileStmt();
        whileStmt->parent = parent;

        auto startToken = eat(TokenType::While);
        eat(TokenType::ParenOpen);
        whileStmt->condition = parseExpr(whileStmt);
        eat(TokenType::ParenClose);
        whileStmt->body = parseStmt(whileStmt);
        return addPosition(whileStmt, startToken);
    }

    Expr* Parser::parseTypeExpr(Node* parent) {
        Expr* expression = nullptr;
        if (match(TokenType::Identifier) || match(TokenType::Null)) {
            auto start = eat();
            auto id = new IdExpr();
            id->parent = parent;
            id->name = start.value;
            expression = addPosition(id, start);
        }
        else {
            expected("Type name");
        }

        while (matchTypeSecondary()) {
            if (match(TokenType::BracketOpen)) {
                auto st = eat();
                eat(TokenType::BracketClose);
                auto arr = new ArrayTypeExpr();
                arr->parent = parent;
                arr->baseTypeExpr = expression;
                arr->baseTypeExpr->parent = arr;
                expression = addPosition(arr, st);
            }
            else if (match(TokenType::LessThan)) {
                expression = parseGenericReificationExpr(parent, expression);
            }
            else if (match(TokenType::Pipe)) {
                auto st = eat();
                auto un = new UnionTypeExpr();
                un->parent = parent;
                un->base = expression;
                un->base->parent = un;
                un->next = parseTypeExpr(un);
                expression = addPosition(un, st);
            }
            else if (match(TokenType::QuestionMark)) {
                auto st = eat();
                auto nullable = new NullableTypeExpr();
                nullable->parent = parent;
                nullable->baseTypeExpr = expression;
                nullable->baseTypeExpr->parent = nullable;
                expression = addPosition(nullable, st);
            }
            else if (match(TokenType::Period)) {
                auto st = eat();
                auto name = eat(TokenType::Identifier);
                auto scope = new ScopeExpr();
                scope->parent = parent;
                scope->name = name.value;
                scope->scopeTarget = expression;
                scope->scopeTarget->parent = scope;
                expression = addPosition(scope, st);
            }
            else {
                eat();
                expected("secondary type expression.");
            }
        }

        return expression;
    }

    GenericReificationExpr* Parser::parseGenericReificationExpr(Node* parent, Expr* target) {
        auto startToken = eat(TokenType::LessThan);
        auto rei = new GenericReificationExpr();
        rei->parent = parent;
        rei->baseTypeExpr = target;
        rei->baseTypeExpr->parent = rei;
        while (!eof() && !match(TokenType::GreaterThan)) {
            rei->genericArguments.push_back(parseTypeExpr(rei));
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::GreaterThan);
        return addPosition(rei, startToken);
    }
    
    Expr* Parser::parseExpr(Node* parent, int precedence) {
        Expr* expression = nullptr;
        if (match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null)) {
            expression = parseLitExpr(parent);
        }
        else if (match(TokenType::Identifier)) {
            expression = parseIdExpr(parent);
        }
        else if (matchUnary()) {
            auto st = eat();
            auto un = new UnaryExpr();
            un->parent = parent;
            un->op = st.type;
            un->target = parseExpr(un, unaryPrecedenceMap[st.type]);
            expression = addPosition(un, st);
        }
        else if (match(TokenType::ParenOpen)) {
            eat();
            expression = parseExpr(parent);
            eat(TokenType::ParenClose);
        }
        else if (match(TokenType::New)) {
            expression = parseNewExpr(parent);
        }
        else if (match(TokenType::This)) {
            auto st = eat();
            auto thisExpr = new ThisExpr();
            thisExpr->parent = parent;
            expression = addPosition(thisExpr, st);
        }
        else if (match(TokenType::BracketOpen)) {
            expression = parseArrayLitExpr(parent);
        }
        else {
            expected("primary expression or prefix operator");
            return nullptr; // TODO return null object
        }

        while (matchSecondary()) {
            auto it = precedenceMap.find(token->type);
            if (it == precedenceMap.end()) {
                error(source, *token, "No precedence for operator '" + token->value + "'");
                eat();
                continue;
            }
            if (it->second <= precedence) {
                break;
            }
            int myprec = it->second;

            if (match(TokenType::ParenOpen)) {
                expression = parseCallExpr(parent, expression);
            }
            else if (match(TokenType::BracketOpen)) {
                expression = parseSubscriptExpr(parent, expression);
            }
            else if (match(TokenType::Period)) {
                auto st = eat();
                auto scope = new ScopeExpr();
                scope->parent = parent;
                scope->name = eat(TokenType::Identifier).value;
                scope->scopeTarget = expression;
                scope->scopeTarget->parent = scope;
                expression = addPosition(scope, st);
            }
            else if (match(TokenType::MinusMinus) || match(TokenType::PlusPlus)) {
                auto st = eat();
                auto post = new PostfixExpr();
                post->parent = parent;
                post->op = st.type;
                post->target = expression;
                post->target->parent = post;
                expression = addPosition(post, st);
            }
            else if (match(TokenType::Is)) {
                auto startToken = eat(TokenType::Is);
                auto is = new IsExpr();
                is->parent = parent;
                is->typeExpr = parseTypeExpr(is);
                is->target = expression;
                is->target->parent = is;
                expression = addPosition(is, startToken);
            }
            else if (match(TokenType::Colon)) {
                auto startToken = eat(TokenType::Colon);
                auto cast = new CastExpr();
                cast->parent = parent;
                cast->sourceExpr = expression;
                cast->sourceExpr->parent = cast;
                cast->targetTypeExpr = parseTypeExpr(cast);
                expression = addPosition(cast, startToken);
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
                    auto assign = new AssignExpr();
                    assign->parent = parent;
                    assign->op = op.type;
                    assign->left = expression;
                    assign->left->parent = assign;
                    assign->right = parseExpr(assign, myprec);
                    expression = addPosition(assign, op);
                }
                else {
                    auto binop = new BinopExpr();
                    binop->parent = parent;
                    binop->op = op.type;
                    binop->left = expression;
                    binop->left->parent = binop;
                    binop->right = parseExpr(binop, myprec);
                    expression = addPosition(binop, op);
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

    NewExpr* Parser::parseNewExpr(Node* parent) {
        auto newExpr = new NewExpr();
        newExpr->parent = parent;

        auto startToken = eat(TokenType::New);
        newExpr->typeExpr = parseTypeExpr(newExpr);
        
        if (eatOptional(TokenType::ParenOpen)) {
            while (!eof() && !match(TokenType::ParenClose)) {
                newExpr->arguments.push_back(parseExpr(newExpr));
                if (!eatOptional(TokenType::Comma)) {
                    break;
                }
            }
            eat(TokenType::ParenClose);
        }
        return addPosition(newExpr, startToken);
    }

    CallExpr* Parser::parseCallExpr(Node* parent, Expr* callTarget) {
        auto call = new CallExpr();
        call->parent = parent;
        call->callTarget = callTarget;

        auto startToken = eat(TokenType::ParenOpen);
        while (!eof() && !match(TokenType::ParenClose)) {
            call->arguments.push_back(parseExpr(call));
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::ParenClose);
        return addPosition(call, startToken);
    }

    SubscriptExpr* Parser::parseSubscriptExpr(Node* parent, Expr* callTarget) {
        auto sub = new SubscriptExpr();
        sub->parent = parent;
        sub->callTarget = callTarget;

        auto startToken = eat(TokenType::BracketOpen);
        while (!eof() && !match(TokenType::BracketClose)) {
            sub->arguments.push_back(parseExpr(sub));
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::BracketClose);
        return addPosition(sub, startToken);
    }

    LitExpr* Parser::parseLitExpr(Node* parent) {
        if (!(match(TokenType::Integer) || match(TokenType::Float) || match(TokenType::String) || match(TokenType::Boolean) || match(TokenType::Null))) {
            expected("literal");
        }
        auto lit = new LitExpr();
        lit->parent = parent;
        auto tok = eat();
        lit->token = tok;
        return addPosition(lit, tok);
    }

    IdExpr* Parser::parseIdExpr(Node* parent) {
        auto id = new IdExpr();
        id->parent = parent;
        auto startToken = eat(TokenType::Identifier);
        id->name = startToken.value;
        return addPosition(id, startToken);
    }

    Param* Parser::parseParam(Node* parent) {
        auto param = new Param();
        param->parent = parent;

        auto startToken = eat(TokenType::Identifier);
        param->name = startToken.value;
        eat(TokenType::Colon);
        param->typeExpr = parseTypeExpr(param);
        return addPosition(param, startToken);
    }

    VarDecl* Parser::parseVarDecl(Node* parent) {
        auto var = new VarDecl();
        var->parent = parent;

        auto startToken = eat(TokenType::Var);
        var->name = eat(TokenType::Identifier).value;

        if (eatOptional(TokenType::Colon)) {
            var->typeExpr = parseTypeExpr(var);
        }

        if (eatOptional(TokenType::Equals)) {
            var->initializer = parseExpr(var);
        }

        if (!var->typeExpr && !var->initializer) {
            expected("type or initializer expression.");
        }

        eat(TokenType::Semicolon);
        auto vardecl = addPosition(var, startToken);
        vardecl->index = numVariables++;
        return vardecl;
    }

    FieldDecl* Parser::parseFieldDecl(Node* parent) {
        auto field = new FieldDecl();
        field->parent = parent;

        auto startToken = eat(TokenType::Var);
        field->name = eat(TokenType::Identifier).value;
        eat(TokenType::Colon);
        field->typeExpr = parseTypeExpr(field);
        eat(TokenType::Semicolon);
        return addPosition(field, startToken);
    }

    ArrayLitExpr* Parser::parseArrayLitExpr(Node* parent) {
        auto arr = new ArrayLitExpr();
        arr->parent = parent;

        auto startToken = eat(TokenType::BracketOpen);
        while (!eof() && !match(TokenType::BracketClose)) {
            arr->elements.push_back(parseExpr(arr));
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::BracketClose);
        return addPosition(arr, startToken);
    }

    TypeAliasDecl* Parser::parseTypeAliasDecl(Node* parent) {
        auto alias = new TypeAliasDecl();
        auto startToken = eat(TokenType::Type);
        alias->_name = eat(TokenType::Identifier).value;
        eat(TokenType::Equals);
        alias->typeExpr = parseTypeExpr(alias);
        eat(TokenType::Semicolon);
        return addPosition(alias, startToken);
    }

    ClassDecl* Parser::parseClassDecl(Node* parent) {
        auto cls = new ClassDecl();
        cls->parent = parent;

        auto startToken = eat(TokenType::Class);
        cls->_name = eat(TokenType::Identifier).value;

        if (eatOptional(TokenType::LessThan)) {
            while (!eof() && !match(TokenType::GreaterThan)) {
                auto tok = eat(TokenType::Identifier);
                auto gen = new GenericParam();
                gen->parent = cls;
                gen->_name = tok.value;
                cls->genericParams.push_back(addPosition(gen, tok));
                if (match(TokenType::Comma)) {
                    eat();
                }
            }
            eat(TokenType::GreaterThan);
        }

        eat(TokenType::CurlyOpen);
        while (!eof() && !match(TokenType::CurlyClose)) {
            if (match(TokenType::Function)) {
                cls->methods.push_back(parseFuncDecl(cls));
            }
            else if (match(TokenType::Var)) {
                auto field = parseFieldDecl(cls);
                field->index = cls->fields.size();
                cls->fields.push_back(field);
            }
            else {
                eat();
                expected("class member declaration");
            }
        }
        eat(TokenType::CurlyClose);
        return addPosition(cls, startToken);
    }

    InterfaceDecl* Parser::parseInterfaceDecl(Node* parent) {
        auto iface = new InterfaceDecl();
        iface->parent = parent;
        auto startToken = eat(TokenType::Interface);
        iface->_name = eat(TokenType::Identifier).value;
        eat(TokenType::CurlyOpen);
        while (!eof() && !match(TokenType::CurlyClose)) {
            auto im = parseInterfaceMethodDecl(iface);
            im->index = iface->methods.size();
            iface->methods.push_back(im);
            eat(TokenType::Semicolon);
        }
        eat(TokenType::CurlyClose);
        return addPosition(iface, startToken);
    }

    InterfaceMethodDecl* Parser::parseInterfaceMethodDecl(Node* parent) {
        auto im = new InterfaceMethodDecl();
        im->parent = parent;
        auto startToken = eat(TokenType::Function);
        im->name = eat(TokenType::Identifier).value;
        eat(TokenType::ParenOpen);
        while (!eof() && !match(TokenType::ParenClose)) {
            auto param = parseParam(im);
            param->index = im->params.size();
            im->params.push_back(param);
            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::ParenClose);
        if (eatOptional(TokenType::Colon)) {
            im->returnTypeExpr = parseTypeExpr(im);
        }
        
        return addPosition(im, startToken);
    }

    EnumDecl* Parser::parseEnumDecl(Node* parent) {
        auto en = new EnumDecl();
        en->parent = parent;
        auto startToken = eat(TokenType::Enum);
        en->_name = eat(TokenType::Identifier).value;
        eat(TokenType::CurlyOpen);
        while (match(TokenType::Identifier)) {
            auto tok = eat(TokenType::Identifier);
            auto ee = new EnumElement();
            ee->parent = en;
            ee->name = tok.value;
            auto el = addPosition(ee, tok);
            el->index = en->elements.size();
            en->elements.push_back(el);

            if (!eatOptional(TokenType::Comma)) {
                break;
            }
        }
        eat(TokenType::CurlyClose);
        return addPosition(en, startToken);
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
            match(TokenType::QuestionMark) ||
            match(TokenType::Period)
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