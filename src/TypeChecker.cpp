#include "TypeChecker.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Types/types.h"
#include "Scope.h"

#include <sstream>

namespace Strela {

    TypeChecker::TypeChecker() {
    }

    void TypeChecker::visit(AstModDecl& n) {
        visitChildren(n.functions);
        visitChildren(n.classes);
    }

    void TypeChecker::visit(AstClassDecl& n) {
        auto oldclass = _class;
        _class = &n;
        
        visitChildren(n.fields);
        visitChildren(n.methods);

        _class = oldclass;
    }

    void TypeChecker::visit(AstFuncDecl& n) {
        auto oldfunction = function;
        function = &n;

        if (n.returnTypeExpr) visitChild(n.returnTypeExpr);
        visitChildren(n.params);
        Type* returnTypeExpr = n.returnTypeExpr ? n.returnTypeExpr->staticValue.type : Types::_void;

        bool unreachableWarning = false;
        for (auto&& stmt: n.stmts) {
            if (n.returns && !unreachableWarning) {
                unreachableWarning = true;
                warning(*stmt, "Unreachable code detected.");
            }
            stmt->accept(*this);
            if (stmt->returns) n.returns = true;
        }

        if (!n.returns && returnTypeExpr != Types::_void) {
            error(n, n.name + ": Not all paths return a value.");
        }

        function = oldfunction;
    }

    void TypeChecker::visit(AstParam& n) {
        visitChild(n.typeExpr);
    }

    void TypeChecker::visit(AstVarDecl& n) {
        if (n.typeExpr) {
            visitChild(n.typeExpr);
        }

        if (n.initializer) {
            visitChild(n.initializer);
            if (n.declType == Types::invalid) {
                n.declType = n.initializer->type;
            }
            if (!n.declType->isAssignableFrom(n.initializer->type)) {
                error(n, n.name + ": Can not assign '" + n.initializer->type->name + "' to '" + n.declType->name + "'.");
            }
        }
    }

    void TypeChecker::visit(AstIdExpr& n) {
        if (n.symbol->kind == SymbolKind::Type) {
            n.type = Types::typetype;
            n.isStatic = true;
            n.staticValue.type = n.symbol->type;
        }
        else {
            if (auto fun = n.symbol->node->as<AstFuncDecl>()) {
                n.isStatic = true;
                auto cur = n.symbol;
                while (cur) {
                    n.candidates.push_back(cur->node->as<AstFuncDecl>());
                    cur = cur->next;
                }
                if (n.candidates.size() == 1) {
                    n.type = n.candidates[0]->declType;
                    n.referencedFunction = n.candidates[0];
                }
                else {
                    n.type = Types::invalid;
                }
            }
            else if (auto param = n.symbol->node->as<AstParam>()) {
                n.type = param->declType;
                n.isStatic = false;
                n.referencedParam = param;
            }
            else if (auto var = n.symbol->node->as<AstVarDecl>()) {
                n.type = var->declType;
                n.isStatic = false;
                n.referencedVar = var;
            }
            else if (auto mod = n.symbol->node->as<AstModDecl>()) {
                n.type = mod->declType;
                n.isStatic = true;
                n.referencedModule = mod;
            }
            else {
                error(n, "Unhandled symbol");
            }
        }
    }

    AstFuncDecl* TypeChecker::findOverload(AstExpr* target, const std::vector<Type*>& argTypes) {
        std::vector<AstFuncDecl*> candidates;
        int numFound = 0;
        for (auto&& candidate: target->candidates) {
            if (candidate->params.size() != argTypes.size()) {
                // mismatching number of arguments
                continue;
            }

            if (candidate->declType->as<FunctionType>()->paramTypes == argTypes) {
                // exact match
                return candidate;
            }

            if (candidate->declType->isCallable(argTypes)) {
                // match with conversion
                candidates.push_back(candidate);
            }
        }

        if (candidates.empty()) {
            error(*target, "No suitable overload found.");
        }

        if (candidates.size() > 1) {
            std::stringstream sstr;
            sstr << "Multiple suitable overloads found. Arguments are (";
            for (auto&& arg: argTypes) {
                sstr << arg->name;
                if (&arg != &argTypes.back()) {
                    sstr << ", ";
                }
            }
            sstr << ").";
            error(*target, sstr.str());
            int i = 0;
            for (auto&& candidate: candidates) {
                ++i;
                error(*candidate, "Candidate " + std::to_string(i) + " is " + candidate->declType->name);
            }
        }

        target->referencedFunction = candidates.front();
        target->type = target->referencedFunction->returnType;
        target->isStatic = true;

        /*if (ft->isCallable(argTypes)) {
            n.type = ft->returnTypeExpr;
        }
        else {
            std::stringstream sstr;
            sstr << "Arguments mismatch. Expected (";
            for (auto&& p: ft->paramTypes) {
                sstr << p->name;
                if (&p != &ft->paramTypes.back()) {
                    sstr << ", ";
                }
            }
            sstr << ") but was called with (";
            for (auto&& a: argTypes) {
                sstr << a->name;
                if (&a != &argTypes.back()) {
                    sstr << ", ";
                }
            }
            sstr << ").";
            error(n, sstr.str());
        }*/

        return candidates.front();
    }

    void TypeChecker::visit(AstCallExpr& n) {
        visitChildren(n.arguments);
        visitChild(n.callTarget);

        if (n.callTarget->candidates.size() > 0) {
            std::vector<Type*> argTypes;
            if (n.callTarget->nodeParent) {
                argTypes.push_back(n.callTarget->nodeParent->type);
            }
            for (auto&& argument: n.arguments) {
                argTypes.push_back(argument->type);
            }

            // find suitable overload
            auto fun = findOverload(n.callTarget, argTypes);
            n.type = fun->returnType;
            n.isStatic = false;
            n.callTarget->referencedFunction = fun;
        }
        else {
            error(n, "Can not call value of type '" + n.callTarget->type->name + "'");
        }
    }

    void TypeChecker::visit(AstRetStmt& n) {
        n.returns = true;

        if (n.expression) {
            visitChild(n.expression);
            if (!function->returnTypeExpr->staticValue.type->isAssignableFrom(n.expression->type)) {
                error(n, function->name + ": Incompatible return type. Returning '" + n.expression->type->name + "' from '" + function->returnTypeExpr->staticValue.type->name + "' function.");
            }
        }
        else if (function->returnType != Types::_void) {
            error(n, "non-void function must return a value.");
        }
    }

    void TypeChecker::visit(AstExprStmt& n) {
        visitChild(n.expression);
    }

    void TypeChecker::visit(AstLitExpr& n) {
        switch (n.token.type) {
            case TokenType::Integer: {
                uint64_t num = std::strtoull(n.token.value.c_str(), nullptr, 10);
                if (num > 0xffffffff) {
                    n.type = Types::u64;
                    n.isStatic = true;
                    n.staticValue.u64 = num;
                }
                else if (num > 0xffff) {
                    n.type = Types::u32;
                    n.isStatic = true;
                    n.staticValue.u32 = num;
                }
                else if (num > 0xff) {
                    n.type = Types::u16;
                    n.isStatic = true;
                    n.staticValue.u16 = num;
                }
                else {
                    n.type = Types::u8;
                    n.isStatic = true;
                    n.staticValue.u8 = num;
                }
                break;
            }

            case TokenType::Float: {
                double num = std::strtod(n.token.value.c_str(), nullptr);
                n.type = Types::f64;
                n.isStatic = true;
                n.staticValue.f64 = num;
                break;
            }

            case TokenType::Boolean:
                n.type = Types::boolean;
                n.isStatic = true;
                n.staticValue.boolean = (n.token.value == "true");
                break;

            case TokenType::String:
                n.type = Types::string;
                n.isStatic = true;
                n.staticValue.string = n.token.value.c_str();
                break;

            case TokenType::Null:
                n.type = Types::null;
                n.isStatic = true;
                break;

            default:
                error(n, "Wrong token type for literal expression.");
        }
    }

    void TypeChecker::visit(AstBlockStmt& n) {
        auto oldblock = block;
        block = &n;
        bool unreachableWarning = false;
        for (auto&& stmt: n.stmts) {
            if (n.returns && !unreachableWarning) {
                unreachableWarning = true;
                warning(*stmt, "Unreachable code detected.");
            }
            visitChild(stmt);
            if (stmt->returns) n.returns = true;
        }
        block = oldblock;
    }

    void TypeChecker::visit(AstBinopExpr& n) {
        visitChild(n.left);
        visitChild(n.right);

        auto ltype = n.left->type;
        auto rtype = n.right->type;

        auto lint = ltype->as<IntType>();
        auto rint = rtype->as<IntType>();
        auto lfloat = ltype->as<FloatType>();
        auto rfloat = rtype->as<FloatType>();

        bool leftScalar = lint || lfloat;
        bool rightScalar = rint || rfloat;

        auto op = n.startToken.type;

        if (
            op == TokenType::Plus ||
            op == TokenType::Minus ||
            op == TokenType::Asterisk ||
            op == TokenType::Slash ||
            op == TokenType::LessThan ||
            op == TokenType::GreaterThan ||
            op == TokenType::LessThanEquals ||
            op == TokenType::GreaterThanEquals
        ) {
            
            if (!leftScalar || !rightScalar) {
                error(n, "Binary operator '" + n.startToken.value + "' is only applicable to scalar values. Types are '" + ltype->name + "' and '" + rtype->name + "'.");
            }

            if (op == TokenType::LessThan || op == TokenType::GreaterThan || op == TokenType::LessThanEquals || op == TokenType::GreaterThanEquals) {
                n.type = Types::boolean;
            }
            else {
                if (lint && rint) {
                    int bytes = lint->bytes > rint->bytes ? lint->bytes : rint->bytes;
                    bool isSigned = lint->isSigned || rint->isSigned;
                    if (isSigned) {
                        if (bytes == 1) n.type = Types::i8;
                        else if (bytes == 2) n.type = Types::i16;
                        else if (bytes == 4) n.type = Types::i32;
                        else if (bytes == 8) n.type = Types::i64;
                    }
                    else {
                        if (bytes == 1) n.type = Types::u8;
                        else if (bytes == 2) n.type = Types::u16;
                        else if (bytes == 4) n.type = Types::u32;
                        else if (bytes == 8) n.type = Types::u64;
                    }
                }
                else if (lfloat && rfloat) {
                    n.type = lfloat->bytes > rfloat->bytes ? lfloat : rfloat;
                }
                else if (lfloat) {
                    n.type = lfloat;
                }
                else if (rfloat) {
                    n.type = rfloat;
                }
            }
        }
        else if (op == TokenType::EqualsEquals || op == TokenType::ExclamationMarkEquals) {
            if (!((leftScalar && rightScalar) || (ltype == rtype))) {
                error(n, "Binary operator '" + n.startToken.value + "' is not applicable to types '" + ltype->name + "' and '" + rtype->name + "'.");
            }
            n.type = Types::boolean;
        }
        else if (op == TokenType::PipePipe || op == TokenType::AmpAmp) {
            if (!(ltype == Types::boolean && rtype == Types::boolean)) {
                error(n, "Binary operator '" + n.startToken.value + "': Both operands must have boolean type. Actual types are '" + ltype->name + "' and '" + rtype->name + "'.");
            }
            n.type = Types::boolean;
        }
        else {
            error(n, "Invalid binary operator '" + n.startToken.value + "'");
        }
    }

    void TypeChecker::visit(AstAssignExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void TypeChecker::visit(AstScopeExpr& n) {
        visitChild(n.scopeTarget);
        auto ttype = n.scopeTarget->type;
        if (auto mod = ttype->as<ModuleType>()) {
            if (auto member = mod->module->getMember(n.name)) {
                if (auto function = member->as<AstFuncDecl>()) {
                    n.isStatic = true;
                    n.candidates = mod->module->getFunctions(n.name);
                    if (n.candidates.size() == 1) {
                        n.type = n.candidates[0]->declType;
                        n.referencedFunction = n.candidates[0];
                    }
                    else {
                        n.type = Types::invalid;
                    }
                }
                else {
                    error(n, "TODO");
                }
            }
            else {
                error(n, "Module '" + mod->name + "' has no member named '" + n.name + "'.");
            }
        }
        else if (auto cls = ttype->as<ClassType>()) {
            if (auto member = cls->_class->getMember(n.name)) {
                if (auto method = member->as<AstFuncDecl>()) {
                    n.isStatic = true;
                    n.nodeParent = n.scopeTarget;
                    n.candidates = cls->_class->getMethods(n.name);
                    if (n.candidates.size() == 1) {
                        n.type = n.candidates[0]->declType;
                        n.referencedFunction = n.candidates[0];
                    }
                    else {
                        n.type = Types::invalid;
                    }
                }
                else if (auto field = member->as<AstFieldDecl>()) {
                    n.type = field->declType;
                    n.isStatic = false;
                    n.nodeParent = n.scopeTarget;
                    n.referencedField = field;
                }
                else {
                    error(n, "TODO");
                }
            }
            else {
                error(n, "Class '" + cls->name + "' has no member named '" + n.name + "'.");
            }
        }
        else if (auto mod = ttype->as<ModuleType>()) {
            if (auto member = mod->module->getMember(n.name)) {
                if (auto method = member->as<AstFuncDecl>()) {
                    n.isStatic = true;
                    n.nodeParent = n.scopeTarget;
                    n.candidates = mod->module->getFunctions(n.name);
                    if (n.candidates.size() == 1) {
                        n.type = n.candidates[0]->declType;
                        n.referencedFunction = n.candidates[0];
                    }
                    else {
                        n.type = Types::invalid;
                    }
                }
                else if (auto field = member->as<AstFieldDecl>()) {
                    n.type = field->declType;
                    n.isStatic = false;
                    n.nodeParent = n.scopeTarget;
                    n.referencedField = field;
                }
                else {
                    error(n, "TODO");
                }
            }
            else {
                error(n, "Module '" + mod->name + "' has no member named '" + n.name + "'.");
            }
        }
        else {
            error(n, "Scope operator is inapplicable to type '" + ttype->name + "'.");
        }
    }

    void TypeChecker::visit(AstIfStmt& n) {
        visitChild(n.condition);
        if (n.condition->type != Types::boolean) {
            error(*n.condition, "Condition must yield boolean value.");
        }
        visitChild(n.trueBranch);
        auto ret = n.trueBranch->returns;
        if (n.falseBranch) {
            visitChild(n.falseBranch);
            n.returns = ret && n.falseBranch->returns;
        }
    }

    void TypeChecker::visit(AstFieldDecl& n) {
        visitChild(n.typeExpr);
    }

    void TypeChecker::visit(AstNewExpr& n) {
        visitChild(n.typeExpr);
        n.type = n.typeExpr->staticValue.type;

        if (!n.type->as<ClassType>()) {
            error(n, "Only class types are instantiable.");
        }
    }

    void TypeChecker::visit(AstIdTypeExpr& n) {
    }

    void TypeChecker::visit(AstWhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void TypeChecker::visit(AstPostfixExpr& n) {
        visitChild(n.target);
        if (!n.target->type->isScalar()) {
            error(n, "Operator '" + n.startToken.value + "' is only applicable to scalar values. Target type is '" + n.target->type->name + "'.");
        }
        if (!(n.target->referencedField || n.target->referencedParam || n.target->referencedVar)) {
            error(n, "Target for operator '" + n.startToken.value + "' must be a reference to a mutable value.");
        }
        n.type = n.target->type;
        n.isStatic = false;
        n.nodeParent = n.target->nodeParent;
        n.referencedField = n.target->referencedField;
        n.referencedVar = n.target->referencedVar;
        n.referencedParam = n.target->referencedParam;
    }

    void TypeChecker::visit(AstArrayTypeExpr& n) {
        visitChild(n.base);
    }

    void TypeChecker::visit(AstImportStmt& n) {
    }

    void TypeChecker::visit(AstUnaryExpr& n) {
        visitChild(n.target);
        switch (n.startToken.type) {
            case TokenType::Minus:
            if (auto intt = n.target->type->as<IntType>()) {
                n.type = intt->getSignedType();
                if (n.target->isStatic) {
                    if (n.target->type == Types::u32) {
                        n.isStatic = true;
                        n.staticValue.i32 = -1 * n.target->staticValue.u32;
                    }
                }
                return;
            }
            if (auto flt = n.target->type->as<FloatType>()) {
                n.type = n.target->type;
                return;
            }
            error(n, "Unary operator '-' is only applicable to numeric types.");
            break;

            case TokenType::ExclamationMark:
            if (n.target->type == Types::boolean) {
                n.type = Types::boolean;
                return;
            }
            error(n, "Unary operator '!' is only applicable to boolean values.");
            break;
        }
        error(n, "Unhandled unary prefix operator '" + n.startToken.value + "'.");
        n.type = Types::invalid;
    }

    void TypeChecker::visit(AstEnumDecl& n) {
        visitChildren(n.elements);
    }

    void TypeChecker::visit(AstEnumElement& n) {
    }

    void TypeChecker::error(AstNode& node, const std::string& msg) {
        //std::cerr << msg << "\n";
        throw TypeException(msg, node);
    }

    void TypeChecker::warning(AstNode& node, const std::string& msg) {
        std::cerr << node.startToken.line << ":" << node.startToken.column << " " << msg << "\n";
    }
}