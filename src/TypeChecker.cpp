// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "TypeChecker.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Scope.h"
#include "SourceFile.h"

#include <sstream>

namespace Strela {

    template<typename T> std::vector<T*> extract(const std::vector<Node*>& nodes) {
        std::vector<T*> result;
        for (auto&& node: nodes) {
            if (auto t = node->as<T>()) {
                result.push_back(t);
            }
        }
        return result;
    }

    std::string operator+(const std::string& left, const std::vector<TypeDecl*>& types) {
        std::string result(left);
        for (auto&& type: types) {
            result += type->name;
            if (&type != &types.back()) {
                result += ", ";
            }
        }
        return result;
    }

    std::vector<TypeDecl*> unique(const std::vector<TypeDecl*>& types) {
        std::set<TypeDecl*> set;
        set.insert(types.begin(), types.end());
        std::vector<TypeDecl*> result;
        result.assign(set.begin(), set.end());
        return result;
    }

    TypeDecl* TypeChecker::getType(Expr* expr) {
        if (expr->node) {
            auto it = refinements.find(expr->node);
            if (it != refinements.end()) {
                for (auto& ref: it->second) {
                    if (ref.type) return ref.type;
                }
            }
        }
        return expr->type;
    }

    std::vector<TypeDecl*> TypeChecker::getTypes(const std::vector<Expr*>& arguments) {
        std::vector<TypeDecl*> result;
        for (auto&& argument: arguments) {
            result.push_back(getType(argument));
        }
        return result;
    }

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
        if (to->as<FloatType>() && (from->as<FloatType>() || from->as<IntType>())) return true;
        if (to->as<BoolType>() && from->as<BoolType>()) return true;
        if (to->as<EnumDecl>() && from == to) return true;
        if (to->as<UnionType>() && from == to) return true;
        if (auto arrTo = to->as<ArrayType>()) {
            if (to == from) return true;
            auto arrFrom = from->as<ArrayType>();
            if (!arrFrom) return false;
            return arrFrom->baseType == arrTo->baseType;
        }
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

    Expr* addCast(Expr* expr, TypeDecl* targetType) {
        auto iface = targetType->as<InterfaceDecl>();
        auto targetunion = targetType->as<UnionType>();
        auto cls = expr->type->as<ClassDecl>();
        
        if (cls && iface) {
            auto cast = new CastExpr(expr, targetType);
            cast->implementation = iface->implementations[cls];
            return cast;
        }
        
        if (targetType != expr->type) {
            return new CastExpr(expr, targetType);
        }
        
        return expr;
    }

    void prepareArguments(std::vector<Expr*>& arguments, FuncType* ftype) {
        for (size_t i = 0; i < ftype->paramTypes.size(); ++i) {
            arguments[i] = addCast(arguments[i], ftype->paramTypes[i]);
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

	std::vector<FuncDecl*> TypeChecker::findOverload(const std::vector<FuncDecl*>& funcs, const std::vector<Expr*>& args) {
        return findOverload(funcs, getTypes(args));
    }

	std::vector<FuncDecl*> TypeChecker::findOverload(const std::vector<FuncDecl*>& candidates, const std::vector<TypeDecl*>& argTypes) {
        std::vector<FuncDecl*> remaining;
        int numFound = 0;
        for (auto&& candidate: candidates) {
            if (candidate->params.size() != argTypes.size()) {
                // mismatching number of arguments
                continue;
            }

            if (candidate->type->as<FuncType>()->paramTypes == argTypes) {
                // exact match
                return {candidate};
            }

            if (isCallable(candidate->type, argTypes)) {
                // match with conversion
                remaining.push_back(candidate);
            }
        }

        return remaining;
    }

    std::vector<FuncDecl*> TypeChecker::findOverload(Expr* target, const std::vector<TypeDecl*>& argTypes) {
        return findOverload(target->candidates, argTypes);
    }

    TypeChecker::TypeChecker() {
    }

    void TypeChecker::visit(ModDecl& n) {
        visitChildren(n.functions);
        visitChildren(n.classes);
    }

    void TypeChecker::visit(IsExpr& n) {
        visitChild(n.target);
        if (auto un = n.target->type->as<UnionType>()) {
            if (un->containedTypes.find(n.typeExpr->type) != un->containedTypes.end()) {
                n.typeTag = un->getTypeTag(n.typeExpr->type);
                if (n.target->node) {
                    refinements[n.target->node].push_back(Refinement{n.target->node, n.typeExpr->type});
                }
                n.type = &BoolType::instance;
            }
            else {
                error(n, "'" + n.typeExpr->type->name + "' is not part of union type '" + un->name + "'.");
            }
        }
        else {
            error(n, "Only union types allowed.");
        }
    }

    void TypeChecker::visit(ClassDecl& n) {
        if (!n.genericParams.empty() && n.genericArguments.empty()) {
            visitChildren(n.reifiedClasses);
        }
        else {
            if (n.genericArguments.size()) genericStack.push_back(&n);

            auto oldclass = _class;
            _class = &n;
            
            visitChildren(n.methods);

            _class = oldclass;

            if (n.genericArguments.size()) genericStack.pop_back();
        }
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

        if (!n.isExternal && !n.returns && n.type->returnType != &VoidType::instance) {
            error(n, n.name + ": Not all paths return a value.");
        }

        function = oldfunction;
    }

    void TypeChecker::visit(ArrayLitExpr& n) {
        visitChildren(n.elements);
        // unify types
        if (!n.elements.empty()) {
            auto types = unique(getTypes(n.elements));
            if (types.size() == 1) {
                n.type = ArrayType::get(types.front());
            }
            else {
                error(n, "Array literal can not contain different types.");
            }
        }
        else {
            error(n, "Empty array literal is not allowed.");
        }
    }

    void TypeChecker::visit(SubscriptExpr& n) {
        visitChildren(n.arguments);
        visitChild(n.callTarget);

        if (auto arr = n.callTarget->type->as<ArrayType>()) {
            if (n.arguments.size() == 1) {
                if (n.arguments.front()->type == &IntType::instance) {
                    n.type = arr->baseType;
                }
                else {
                    error(n, "Array subscript operator argument must be integer.");
                }
            }
            else {
                error(n, "Array subscript operator takes one argument.");
            }
        }
        else {
            auto candidates = extract<FuncDecl>(n.callTarget->type->getMethods("[]"));
            auto funs = findOverload(candidates, n.arguments);
            if (funs.empty()) {
                error(n, "Type '" + n.callTarget->type->name + "' has no subscript operator with arguments (" + getTypes(n.arguments) + ").");
            }
            else if (funs.size() > 1) {
                error(n, "Type '" + n.callTarget->type->name + "' has multiple matching subscript operators for arguments (" + getTypes(n.arguments) + ").");
            }
            else {
                auto fun = funs.front();
                n.type = fun->type->returnType;
                n.subscriptFunction = fun;
            }
        }
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
        visitChildren(n.arguments);
        visitChild(n.callTarget);


        // gather argument types
        auto argTypes = getTypes(n.arguments);

        if (auto ftype = n.callTarget->type->as<FuncType>()) {
            // non-overloaded function call
            if (isCallable(ftype, argTypes)) {
                n.type = ftype->returnType;
                prepareArguments(n.arguments, ftype);
            }
            else {
                std::string msg = "'" + n.callTarget->type->name + "' is not callable with arguments (" + argTypes + ")";
                error(*n.callTarget, msg);
            }
        }
        else if (n.callTarget->candidates.size() > 0) {
            // overloaded function call
            auto funs = findOverload(n.callTarget, argTypes);
            if (funs.size() == 1) {
                auto fun = funs.front();
                n.callTarget->type = fun->type;
                n.callTarget->node = fun;
                n.callTarget->candidates.clear();
                n.type = fun->type->returnType;
                prepareArguments(n.arguments, fun->type);
            }
            else if (funs.empty()) {
                error(n, "'" + n.callTarget->candidates.front()->name + "': No matching overload found. arguments are: (" + argTypes + ")");
            }
            else {
                error(n, "Multiple matching overloads found.");
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
            
            if (!((leftScalar && rightScalar) || (op == TokenType::Plus && ltype == &ClassDecl::String && rtype == &ClassDecl::String))) {
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
                else if (ltype == &ClassDecl::String) {
                    n.type = &ClassDecl::String;
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
        visitChild(n.left);

        if (n.op != TokenType::Equals) {
            TokenType op;
            switch (n.op) {
                case TokenType::PlusEquals: op = TokenType::Plus; break;
                case TokenType::MinusEquals: op = TokenType::Minus; break;
                case TokenType::AsteriskEquals: op = TokenType::Asterisk; break;
                case TokenType::SlashEquals: op = TokenType::Slash; break;
                default: error(n, "invalid op");
            }
            n.right = new BinopExpr(op, n.left, n.right);
        }
        visitChild(n.right);

        if (!n.left->node) {
            error(n, "Assignment target must be storage node.");
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

        n.right = addCast(n.right, n.left->type);
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
                    error(n, "Module '" + mod->name + "' has no member named '" + n.name + "'");
                }
            }
            else if (auto en = n.scopeTarget->node->as<EnumDecl>()) {
                n.node = en->getMember(n.name);
                if (n.node) {
                    n.type = en;
                }
                else {
                    error(n, "Enum '" + en->name + "' has no element named '" + n.name + "'");
                }
            }
            else {
                error(n, "Direct member acces only for modules and enums.");
            }
        }
        else if (auto type = n.scopeTarget->type->as<TypeDecl>()) {
            n.node = type->getMember(n.name);
            n.context = n.scopeTarget;
            if (n.node) {
                if (auto field = n.node->as<FieldDecl>()) {
                    n.type = field->type;
                }
                else if (auto fun = n.node->as<FuncDecl>()) {
                    n.type = fun->type;
                }
                else if (auto met = n.node->as<InterfaceMethodDecl>()) {
                    n.type = met->type;
                }
                else {
                    error(n, "unhandled member kind");
                }
            }
            else {
                error(n, "Type '" + type->name + "' has no member named '" + n.name + "'");
            }
        }        
        else {
            error(n, "Scope operator not applicable to type '" + n.scopeTarget->type->name + "'.");
        }
    }

    void TypeChecker::visit(IfStmt& n) {
        refinements.clear();
        visitChild(n.condition);
        if (n.condition->type != &BoolType::instance) {
            error(*n.condition, "Condition must yield boolean value.");
        }
        visitChild(n.trueBranch);
        auto ret = n.trueBranch->returns;
        if (n.falseBranch) {
            negateRefinements();
            visitChild(n.falseBranch);
            n.returns = ret && n.falseBranch->returns;
        }
    }

    void TypeChecker::negateRefinements() {
        std::map<Node*, std::vector<Refinement>> newRefinements;
        for (auto& pair: refinements) {
            auto& node = pair.first;
            for (auto&& ref: pair.second) {
                if (ref.type) {
                    if (auto var = ref.node->as<VarDecl>()) {
                        if (auto uni = var->type->as<UnionType>()) {
                            if (uni->containedTypes.size() == 2) {
                                newRefinements[node].push_back(Refinement{node, uni->getComplementaryType(ref.type)});
                            }
                        }
                    }
                    else if (auto par = ref.node->as<Param>()) {
                        if (auto uni = par->type->as<UnionType>()) {
                            if (uni->containedTypes.size() == 2) {
                                newRefinements[node].push_back(Refinement{node, uni->getComplementaryType(ref.type)});
                            }
                        }
                    }
                }
            }
        }
        refinements = newRefinements;
    }

    void TypeChecker::visit(NewExpr& n) {
        visitChildren(n.arguments);

        if (auto cls = n.typeExpr->type->as<ClassDecl>()) {
            n.type = cls;

            auto inits = extract<FuncDecl>(cls->getMethods("init"));
            if (inits.empty()) {
                if (!n.arguments.empty()) {
                    error(n, "'" + cls->name + "' has no matching constructor for arguments (" + getTypes(n.arguments) + ").");
                }
            }
            else {
                auto init = findOverload(inits, n.arguments);
                if (init.size() == 1) {
                    n.initMethod = init.front();
                }
                else if (init.empty()) {
                    error(n, "No matching constructors found for arguments (" + getTypes(n.arguments) + ").");
                }
                else {
                    error(n, "Multiple matching constructors found for arguments (" + getTypes(n.arguments) + ").");
                }
            }
        }
        else {
            error(n, "Only class types are instantiable, type is '" + n.typeExpr->type->name + "'");
        }
    }

    void TypeChecker::visit(WhileStmt& n) {
        visitChild(n.condition);
        visitChild(n.body);
    }

    void TypeChecker::visit(PostfixExpr& n) {
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
        //throw TypeException(node.source->filename + ":" + std::to_string(node.line) + ":" + std::to_string(node.column) + " Error: " + msg);
        std::cerr << "\033[1;31m" << node.source->filename << ":" << node.line << ":" << node.column << " Error: " << msg << "\033[0m\n";
        for (auto& gs: genericStack) {
            std::cerr << "Info: During reification of " << gs->name << "\n";
        }

        _hasErrors = true;
    }

    void TypeChecker::warning(Node& node, const std::string& msg) {
        std::cerr << node.source->filename << ":" << node.line << ":" << node.column << " " << msg << "\n";
    }
}