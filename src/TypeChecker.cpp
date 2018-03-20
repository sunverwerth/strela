#include "TypeChecker.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Scope.h"
#include "SourceFile.h"

#include <sstream>

namespace Strela {

    bool isCallable(TypeDecl* ftype, const std::vector<TypeDecl*>& argTypes);
    bool isAssignableFrom(TypeDecl* to, TypeDecl* from);

    Implementation* getImplementation(ClassDecl* cls, InterfaceDecl* iface) {
        auto it = iface->implementations.find(cls);
        if (it != iface->implementations.end()) return it->second;

        std::vector<FuncDecl*> classMethods;
        for (auto&& imethod: iface->methods) {
            bool found = false;
            for (auto&& method: cls->methods) {
                if (method->name == imethod->name && isCallable(method->type, imethod->type->paramTypes)) {
                    found = true;
                    classMethods.push_back(method);
                    break;
                }
            }
            if (!found) return nullptr;
        }
        auto implementation = new Implementation{iface, cls, classMethods};
        iface->implementations.insert(std::make_pair(cls, implementation));
        return implementation;
    }

    bool isAssignableFrom(TypeDecl* to, TypeDecl* from) {
        if (to->as<ClassDecl>() && to == from) return true;
        if (to->as<IntType>() && from->as<IntType>()) return true;
        if (to->as<FloatType>() && from->as<FloatType>()) return true;
        if (to->as<BoolType>() && from->as<BoolType>()) return true;
        if (to->as<EnumDecl>() && from == to) return true;
        if (auto iface = to->as<InterfaceDecl>()) {
            if (to == from) return true;
            auto cls = from->as<ClassDecl>();
            if (!cls) return false;
            return getImplementation(cls, iface) != nullptr;
        }
        return false;
    }

    bool isCallable(TypeDecl* ftype, const std::vector<TypeDecl*>& argTypes) {
        auto signature = ftype->as<FuncType>();
        if (!signature) return false;

        if (signature->paramTypes.size() != argTypes.size()) return false;

        for (size_t i = 0; i < argTypes.size(); ++i) {
            if (!isAssignableFrom(signature->paramTypes[i], argTypes[i])) return false;
        }
        return true;
    }

    bool isScalar(TypeDecl* type) {
        return
            type->as<FloatType>() ||
            type->as<IntType>()
            ;
    }

    void prepareArguments(std::vector<Expr*>& arguments, FuncType* ftype) {
        for (size_t i = 0; i < ftype->paramTypes.size(); ++i) {
            auto iface = ftype->paramTypes[i]->as<InterfaceDecl>();
            auto cls = arguments[i]->type->as<ClassDecl>();
            if (cls && iface) {
                auto cast = new CastExpr(arguments[i], ftype->paramTypes[i]);
                cast->implementation = iface->implementations[cls];
                arguments[i] = cast;
            }
        }
    }


    TypeDecl* getSignedType(TypeDecl* t) {
        if (auto intt = t->as<IntType>()) {
            return intt;
        }
        else if (auto floatt = t->as<FloatType>()) {
            return floatt;
        }
        else {
            return &InvalidType::instance;
        }
    }

	FuncDecl* TypeChecker::findOverload(const std::vector<FuncDecl*>& funcs, const std::vector<Expr*>& args) {
        std::vector<TypeDecl*> argTypes;
        for (auto&& arg: args) {
            argTypes.push_back(arg->type);
        }
        return findOverload(funcs, argTypes);
    }

	FuncDecl* TypeChecker::findOverload(const std::vector<FuncDecl*>& candidates, const std::vector<TypeDecl*>& argTypes) {
        std::vector<FuncDecl*> remaining;
        int numFound = 0;
        for (auto&& candidate: candidates) {
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
                remaining.push_back(candidate);
            }
        }

        if (remaining.empty()) {
            error(*candidates.front(), "No suitable overload found.");
        }

        if (remaining.size() > 1) {
            std::stringstream sstr;
            sstr << "Multiple suitable overloads found. Arguments are (";
            for (auto&& arg: argTypes) {
                sstr << arg->name;
                if (&arg != &argTypes.back()) {
                    sstr << ", ";
                }
            }
            sstr << ").";
            error(*candidates.front(), sstr.str());
            int i = 0;
            for (auto&& candidate: remaining) {
                ++i;
                error(*candidate, "Candidate " + std::to_string(i) + " is " + candidate->type->name);
            }
        }

        return remaining.front();
    }

    FuncDecl* TypeChecker::findOverload(Expr* target, const std::vector<TypeDecl*>& argTypes) {
        return findOverload(target->candidates, argTypes);
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
        
        visitChildren(n.methods);

        _class = oldclass;
    }

    void TypeChecker::visit(FuncDecl& n) {
        auto oldfunction = function;
        function = &n;

        bool unreachableWarning = false;
        for (auto&& stmt: n.stmts) {
            if (n.returns && !unreachableWarning) {
                unreachableWarning = true;
                warning(*stmt, "Unreachable code detected.");
            }
            stmt->accept(*this);
            if (stmt->returns) n.returns = true;
        }

        if (!n.returns && n.type->returnType != &VoidType::instance) {
            error(n, n.name + ": Not all paths return a value.");
        }

        function = oldfunction;
    }

    void TypeChecker::visit(VarDecl& n) {
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

    void TypeChecker::visit(CallExpr& n) {
        n.type = &InvalidType::instance;

        visitChildren(n.arguments);
        visitChild(n.callTarget);


        // gather argument types
        std::vector<TypeDecl*> argTypes;
        for (auto&& argument: n.arguments) {
            argTypes.push_back(argument->type);
        }

        if (auto ftype = n.callTarget->type->as<FuncType>()) {
            // non-overloaded function call
            if (isCallable(ftype, argTypes)) {
                n.type = ftype->returnType;
                prepareArguments(n.arguments, ftype);
            }
            else {
                std::string msg = "'" + n.callTarget->type->name + "' is not callable with arguments (";
                for (auto&& argType: argTypes) {
                    msg += argType->name;
                    if (&argType != &argTypes.back()) {
                        msg += ", ";
                    }
                }
                msg += ")";
                error(*n.callTarget, msg);
            }
        }
        else if (n.callTarget->candidates.size() > 0) {
            // overloaded function call
            auto fun = findOverload(n.callTarget, argTypes);
            if (fun) {
                n.callTarget->type = fun->type;
                n.callTarget->node = fun;
                n.callTarget->candidates.clear();
                n.type = fun->type->returnType;
                prepareArguments(n.arguments, fun->type);
            }
            else {
                error(n, "No matching overload found.");
            }
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
        n.type = &InvalidType::instance;

        switch (n.token.type) {
            case TokenType::Integer: {
                n.type = &IntType::instance;
                break;
            }

            case TokenType::Float: {
                n.type = &FloatType::instance;
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
        n.type = &InvalidType::instance;

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

        auto op = n.op;

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
                error(n, "Binary operator '" + getTokenName(op) + "' is only applicable to scalar values. Types are '" + ltype->name + "' and '" + rtype->name + "'.");
            }

            if (op == TokenType::LessThan || op == TokenType::GreaterThan || op == TokenType::LessThanEquals || op == TokenType::GreaterThanEquals) {
                n.type = &BoolType::instance;
            }
            else {
                if (lint && rint) {
                    n.type = &IntType::instance;
                }
                else if (lfloat && rfloat) {
                    n.type = lfloat;
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
                error(n, "Binary operator '" + getTokenName(op) + "' is not applicable to types '" + ltype->name + "' and '" + rtype->name + "'.");
            }
            n.type = &BoolType::instance;
        }
        else if (op == TokenType::PipePipe || op == TokenType::AmpAmp) {
            if (!(ltype == &BoolType::instance && rtype == &BoolType::instance)) {
                error(n, "Binary operator '" + getTokenName(op) + "': Both operands must have boolean type. Actual types are '" + ltype->name + "' and '" + rtype->name + "'.");
            }
            n.type = &BoolType::instance;
        }
        else {
            error(n, "Invalid binary operator '" + getTokenName(op) + "'");
        }
    }

    void TypeChecker::visit(AssignExpr& n) {
        n.type = &InvalidType::instance;

        visitChild(n.left);
        visitChild(n.right);

        if (!n.left->node) {
            error(n, "Assignment target must be storage node.");
            n.type = &InvalidType::instance;
            return;
        }
        else if (auto var = n.left->node->as<VarDecl>()) {
            n.type = var->type;
        }
        else if (auto param = n.left->node->as<Param>()) {
            n.type = param->typeExpr->type;
        }
        else if (auto field = n.left->node->as<FieldDecl>()) {
            n.type = field->typeExpr->type;
        }
        else {
            error(n, std::string("Can not assign to ") + n.left->getTypeInfo()->getName());
            return;
        }
    }

    void TypeChecker::visit(IdExpr& n) {
        if (n.node) {
            if (auto var = n.node->as<VarDecl>()) {
                n.type = var->type;
            }
            else if (auto fun = n.node->as<FuncDecl>()) {
                n.type = fun->type;
            }
        }
    }

    void TypeChecker::visit(ScopeExpr& n) {
        n.type = &InvalidType::instance;
        visitChild(n.scopeTarget);

        if (n.scopeTarget->type->as<TypeType>()) {
            if (auto mod = n.scopeTarget->node->as<ModDecl>()) {
                n.node = mod->getMember(n.name);
                if (n.node) {
                    if (n.node->as<TypeDecl>()) {
                        n.type = &TypeType::instance;
                    }
                    else if (auto f = n.node->as<FuncDecl>()) {
                        n.type = f->type;
                    }
                    else {
                        error(n, "unhandled member kind");
                    }
                }
                else {
                    error(n, "no member");
                }
            }
            else if (auto en = n.scopeTarget->node->as<EnumDecl>()) {
                n.node = en->getMember(n.name);
                n.type = n.node ? (TypeDecl*)en : (TypeDecl*)&InvalidType::instance;
            }
            else {
                error(n, "Direct member acces only for modules and enums.");
            }
        }
        else if (auto cls = n.scopeTarget->type->as<ClassDecl>()) {
            n.node = cls->getMember(n.name);
            n.context = n.scopeTarget;
            if (n.node) {
                if (auto field = n.node->as<FieldDecl>()) {
                    n.type = field->type;
                }
                else if (auto fun = n.node->as<FuncDecl>()) {
                    n.type = fun->type;
                }
                else {
                    error(n, "unhandled member kind");
                }
            }
            else {
                error(n, "no member");
            }
        }
        else if (auto iface = n.scopeTarget->type->as<InterfaceDecl>()) {
            n.node = iface->getMember(n.name);
            n.context = n.scopeTarget;
            if (n.node) {
                if (auto met = n.node->as<InterfaceMethodDecl>()) {
                    n.type = met->type;
                }
                else {
                    error(n, "unhandled member kind");
                }
            }
            else {
                error(n, "no member");
            }
        }
        else {
            error(n, "Scope operator not applicable to '" + n.scopeTarget->type->name + "'.");
        }
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

    void TypeChecker::visit(NewExpr& n) {
        n.type = &InvalidType::instance;
        if (auto cls = n.typeExpr->type->as<ClassDecl>()) {
            n.type = cls;

            auto inits = cls->getMethods("init");
            if (inits.empty()) {
                if (!n.arguments.empty()) {
                    error(n, "'" + cls->name + "' has no matching constructor.");
                }
            }
            else {
                auto init = findOverload(inits, n.arguments);
                n.initMethod = init;
            }
        }
        else {
            error(n, "Only class types are instantiable.");
        }
    }

    void TypeChecker::visit(WhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void TypeChecker::visit(PostfixExpr& n) {
        n.type = &InvalidType::instance;
        visitChild(n.target);
        if (!n.target->node || !(n.target->node->as<FieldDecl>() || n.target->node->as<Param>() || n.target->node->as<VarDecl>())) {
            error(n, "Target for operator '" + getTokenName(n.op) + "' must be a reference to a mutable value.");
            return;
        }
        if (!isScalar(n.target->type)) {
            error(n, "Operator '" + getTokenName(n.op) + "' is only applicable to scalar values. Target type is '" + n.target->type->name + "'.");
            return;
        }
        n.type = n.target->type;
        n.node = n.target->node;
    }

    void TypeChecker::visit(UnaryExpr& n) {
        n.type = &InvalidType::instance;
        visitChild(n.target);
        switch (n.op) {
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
        error(n, "Unhandled unary prefix operator '" + getTokenName(n.op) + "'.");
        n.type = &InvalidType::instance;
    }

    void TypeChecker::visit(InterfaceDecl&) {
    }

    void TypeChecker::visit(InterfaceMethodDecl&) {
    }

    void TypeChecker::visit(ThisExpr& n) {
        if (_class) {
            n._class = _class;
            n.type = _class;
        }
        else {
            error(n, "'this' expression outside of class.");
        }
    }



    void TypeChecker::error(Node& node, const std::string& msg) {
        //std::cerr << msg << "\n";
        throw TypeException(node.source->filename + ":" + std::to_string(node.line) + ":" + std::to_string(node.column) + " Error: " + msg);
    }

    void TypeChecker::warning(Node& node, const std::string& msg) {
        std::cerr << node.source->filename << ":" << node.line << ":" << node.column << " " << msg << "\n";
    }
}