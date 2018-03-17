#include "TypeChecker.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Scope.h"

#include <sstream>

namespace Strela {

    bool isAssignableFrom(const TypeDecl* to, const TypeDecl* from) {
        if (to->as<ClassDecl>() && to == from) return true;
        if (to->as<IntType>() && from->as<IntType>()) return true;
        if (to->as<FloatType>() && from->as<FloatType>()) return true;
        if (to->as<BoolType>() && from->as<BoolType>()) return true;
        return false;
    }

    bool isCallable(const TypeDecl* ftype, const std::vector<TypeDecl*>& argTypes) {
        auto signature = ftype->as<FuncType>();
        if (!signature) return false;

        if (signature->paramTypes.size() != argTypes.size()) return false;

        for (size_t i = 0; i < argTypes.size(); ++i) {
            if (!isAssignableFrom(signature->paramTypes[i], argTypes[i])) return false;
        }
        return true;
    }

    bool isScalar(const TypeDecl* type) {
        return
            type->as<FloatType>() ||
            type->as<IntType>()
            ;
    }

    TypeDecl* getSignedType(TypeDecl* t) {
        if (auto intt = t->as<IntType>()) {
            if (intt->isSigned) return intt;
            else if (intt->bytes == 1) return &IntType::i8;
            else if (intt->bytes == 2) return &IntType::i16;
            else if (intt->bytes == 4) return &IntType::i32;
            else if (intt->bytes == 8) return &IntType::i64;
        }
        else if (auto floatt = t->as<FloatType>()) {
            return floatt;
        }
        else {
            return &InvalidType::instance;
        }
    }

    TypeChecker::TypeChecker() {
    }

    void TypeChecker::visit(ModDecl& n) {
        visitChildren(n.functions);
        visitChildren(n.classes);
    }

    void TypeChecker::visit(ClassDecl& n) {
        auto oldclass = _class;
        _class = &n;
        
        visitChildren(n.fields);
        visitChildren(n.methods);

        _class = oldclass;
    }

    void TypeChecker::visit(FuncDecl& n) {
        auto oldfunction = function;
        function = &n;

        visitChild(n.returnTypeExpr);
        visitChildren(n.params);
        TypeDecl* returnType = n.returnTypeExpr->type;

        bool unreachableWarning = false;
        for (auto&& stmt: n.stmts) {
            if (n.returns && !unreachableWarning) {
                unreachableWarning = true;
                warning(*stmt, "Unreachable code detected.");
            }
            stmt->accept(*this);
            if (stmt->returns) n.returns = true;
        }

        if (!n.returns && returnType != &VoidType::instance) {
            error(n, n.name + ": Not all paths return a value.");
        }

        function = oldfunction;
    }

    void TypeChecker::visit(Param& n) {
        visitChild(n.typeExpr);
    }

    void TypeChecker::visit(VarDecl& n) {
        if (n.typeExpr) {
            visitChild(n.typeExpr);
        }

        if (n.initializer) {
            visitChild(n.initializer);
            if (n.typeExpr == nullptr) {
                n.type = n.initializer->type;
            }
            if (!isAssignableFrom(n.type, n.initializer->type)) {
                error(n, n.name + ": Can not assign '" + n.initializer->type->name + "' to '" + n.type->name + "'.");
            }
        }
    }

    void TypeChecker::visit(IdExpr& n) {
        if (auto fun = n.symbol->node->as<FuncDecl>()) {
            if (n.symbol->next) {
                auto cur = n.symbol;
                while (cur) {
                    n.candidates.push_back(cur->node->as<FuncDecl>());
                    cur = cur->next;
                }
                n.type = &OverloadedFuncType::instance;
            }
            else {
                n.type = fun->type;
                n.node = fun;
            }
        }
        else if (auto param = n.symbol->node->as<Param>()) {
            n.type = param->typeExpr->type;
            n.node = param;
        }
        else if (auto var = n.symbol->node->as<VarDecl>()) {
            n.type = var->type;
            n.node = var;
        }
        else if (auto td = n.symbol->node->as<TypeDecl>()) {
            n.type = &TypeType::instance;
            n.node = td;
        }
        else {
            error(n, "Unhandled symbol");
        }
    }

    FuncDecl* TypeChecker::findOverload(Expr* target, const std::vector<TypeDecl*>& argTypes) {
        std::vector<FuncDecl*> candidates;
        int numFound = 0;
        for (auto&& candidate: target->candidates) {
            if (candidate->params.size() != argTypes.size()) {
                // mismatching number of arguments
                continue;
            }

            if (candidate->type->as<FuncType>()->paramTypes == argTypes) {
                // exact match
                return candidate;
            }

            if (isCallable(candidate->type, argTypes)) {
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
                error(*candidate, "Candidate " + std::to_string(i) + " is " + candidate->type->name);
            }
        }

        return candidates.front();
    }

    void TypeChecker::visit(CallExpr& n) {
        visitChildren(n.arguments);
        visitChild(n.callTarget);

        std::vector<TypeDecl*> argTypes;
        if (auto scope = n.callTarget->as<ScopeExpr>()) {
            argTypes.push_back(scope->scopeTarget->type);
        }
        for (auto&& argument: n.arguments) {
            argTypes.push_back(argument->type);
        }

        if (n.callTarget->node) {
            if (isCallable(n.callTarget->node->as<FuncDecl>()->type, argTypes)) {
                n.type = n.callTarget->type->as<FuncType>()->returnType;
            }
            else {
                error(*n.callTarget, "Is not callable");
            }
        }
        else if (n.callTarget->candidates.size() > 0) {
            // find suitable overload
            auto fun = findOverload(n.callTarget, argTypes);
            n.callTarget->type = fun->type;
            n.callTarget->node = fun;

            n.type = fun->type->returnType;
        }
        else {
            error(n, "Can not call value of type '" + n.callTarget->type->name + "'");
        }
    }

    void TypeChecker::visit(RetStmt& n) {
        n.returns = true;

        if (n.expression) {
            visitChild(n.expression);
            if (!isAssignableFrom(function->type->returnType, n.expression->type)) {
                error(n, function->name + ": Incompatible return type. Returning '" + n.expression->type->name + "' from '" + function->returnTypeExpr->type->name + "' function.");
            }
        }
        else if (function->type->returnType != &VoidType::instance) {
            error(n, "non-void function must return a value.");
        }
    }

    void TypeChecker::visit(ExprStmt& n) {
        visitChild(n.expression);
    }

    void TypeChecker::visit(LitExpr& n) {
        switch (n.token.type) {
            case TokenType::Integer: {
                uint64_t num = std::strtoull(n.token.value.c_str(), nullptr, 10);
                if (num > 0xffffffff) {
                    n.type = &IntType::u64;
                }
                else if (num > 0xffff) {
                    n.type = &IntType::u32;
                }
                else if (num > 0xff) {
                    n.type = &IntType::u16;
                }
                else {
                    n.type = &IntType::u8;
                }
                break;
            }

            case TokenType::Float: {
                double num = std::strtod(n.token.value.c_str(), nullptr);
                n.type = &FloatType::f64;
                break;
            }

            case TokenType::Boolean:
                n.type = &BoolType::instance;
                break;

            case TokenType::String:
                n.type = &ClassDecl::String;
                break;

            case TokenType::Null:
                n.type = &NullType::instance;
                break;

            default:
                error(n, "Wrong token type for literal expression.");
        }
    }

    void TypeChecker::visit(BlockStmt& n) {
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

    void TypeChecker::visit(BinopExpr& n) {
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
                n.type = &BoolType::instance;
            }
            else {
                if (lint && rint) {
                    int bytes = lint->bytes > rint->bytes ? lint->bytes : rint->bytes;
                    bool isSigned = lint->isSigned || rint->isSigned;
                    if (isSigned) {
                        if (bytes == 1) n.type = &IntType::i8;
                        else if (bytes == 2) n.type = &IntType::i16;
                        else if (bytes == 4) n.type = &IntType::i32;
                        else if (bytes == 8) n.type = &IntType::i64;
                    }
                    else {
                        if (bytes == 1) n.type = &IntType::u8;
                        else if (bytes == 2) n.type = &IntType::u16;
                        else if (bytes == 4) n.type = &IntType::u32;
                        else if (bytes == 8) n.type = &IntType::u64;
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
            n.type = &BoolType::instance;
        }
        else if (op == TokenType::PipePipe || op == TokenType::AmpAmp) {
            if (!(ltype == &BoolType::instance && rtype == &BoolType::instance)) {
                error(n, "Binary operator '" + n.startToken.value + "': Both operands must have boolean type. Actual types are '" + ltype->name + "' and '" + rtype->name + "'.");
            }
            n.type = &BoolType::instance;
        }
        else {
            error(n, "Invalid binary operator '" + n.startToken.value + "'");
        }
    }

    void TypeChecker::visit(AssignExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
    }

    void TypeChecker::visit(ScopeExpr& n) {
        visitChild(n.scopeTarget);        
    }

    void TypeChecker::visit(IfStmt& n) {
        visitChild(n.condition);
        if (n.condition->type != &BoolType::instance) {
            error(*n.condition, "Condition must yield boolean value.");
        }
        visitChild(n.trueBranch);
        auto ret = n.trueBranch->returns;
        if (n.falseBranch) {
            visitChild(n.falseBranch);
            n.returns = ret && n.falseBranch->returns;
        }
    }

    void TypeChecker::visit(FieldDecl& n) {
        visitChild(n.typeExpr);
    }

    void TypeChecker::visit(NewExpr& n) {
        visitChild(n.typeExpr);
        n.type = n.typeExpr->type;

        if (!n.type->as<ClassDecl>()) {
            error(n, "Only class types are instantiable.");
        }
    }

    void TypeChecker::visit(IdTypeExpr& n) {
    }

    void TypeChecker::visit(WhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void TypeChecker::visit(PostfixExpr& n) {
        visitChild(n.target);
        if (!n.target->node || !(n.target->node->as<FieldDecl>() || n.target->node->as<Param>() || n.target->node->as<VarDecl>())) {
            error(n, "Target for operator '" + n.startToken.value + "' must be a reference to a mutable value.");
            return;
        }
        if (!isScalar(n.target->type)) {
            error(n, "Operator '" + n.startToken.value + "' is only applicable to scalar values. Target type is '" + n.target->type->name + "'.");
            return;
        }
        n.type = n.target->type;
        n.node = n.target->node;
    }

    void TypeChecker::visit(ArrayTypeExpr& n) {
        visitChild(n.base);
    }

    void TypeChecker::visit(ImportStmt& n) {
    }

    void TypeChecker::visit(UnaryExpr& n) {
        visitChild(n.target);
        switch (n.startToken.type) {
            case TokenType::Minus:
            if (auto intt = n.target->type->as<IntType>()) {
                n.type = getSignedType(intt);
                return;
            }
            if (auto flt = n.target->type->as<FloatType>()) {
                n.type = n.target->type;
                return;
            }
            error(n, "Unary operator '-' is only applicable to numeric types.");
            break;

            case TokenType::ExclamationMark:
            if (n.target->type == &BoolType::instance) {
                n.type = &BoolType::instance;
                return;
            }
            error(n, "Unary operator '!' is only applicable to boolean values.");
            break;
        }
        error(n, "Unhandled unary prefix operator '" + n.startToken.value + "'.");
        n.type = &InvalidType::instance;
    }

    void TypeChecker::visit(EnumDecl& n) {
        visitChildren(n.elements);
    }

    void TypeChecker::visit(EnumElement& n) {
    }

    void TypeChecker::error(Node& node, const std::string& msg) {
        //std::cerr << msg << "\n";
        throw TypeException(msg, node);
    }

    void TypeChecker::warning(Node& node, const std::string& msg) {
        std::cerr << node.startToken.line << ":" << node.startToken.column << " " << msg << "\n";
    }
}