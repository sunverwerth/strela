// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "TypeChecker.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "Scope.h"
#include "SourceFile.h"
#include "utils.h"

#include <sstream>

namespace Strela {



    std::string operator+(const std::string& left, const std::vector<TypeDecl*>& types) {
        std::string result(left);
        for (auto&& type: types) {
            result += type->getFullName();
            if (&type != &types.back()) {
                result += ", ";
            }
        }
        return result;
    }

    template<typename T> std::vector<T*> extract(const std::vector<Node*>& elements) {
        std::vector<T*> result;
        for (auto&& el: elements) {
            if (auto t = el->as<T>()) {
                result.push_back(t);
            }
        }
        return result;
    }    

    TypeDecl* TypeChecker::getType(Expr* expr) {
        if (expr->node) {
            for (auto it = refinements.rbegin(); it != refinements.rend(); ++it) {
                auto r = it->find(expr->node);
                if (r != it->end()) {
                    for (auto& ref: r->second) {
                        if (ref.type) return ref.type;
                    }
                }
            }
        }
        return expr->type;
    }

    TypeDecl* TypeChecker::getType(VarDecl* var) {
        for (auto it = refinements.rbegin(); it != refinements.rend(); ++it) {
            auto r = it->find(var);
            if (r != it->end()) {
                for (auto& ref: r->second) {
                    if (ref.type) return ref.type;
                }
            }
        }
        return var->declType;
    }

    TypeDecl* TypeChecker::getType(Param* param) {
        for (auto it = refinements.rbegin(); it != refinements.rend(); ++it) {
            auto r = it->find(param);
            if (r != it->end()) {
                for (auto& ref: r->second) {
                    if (ref.type) return ref.type;
                }
            }
        }
        return param->declType;
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
                if (method->name == imethod->name && isCallable(method->declType, imethod->type->paramTypes)) {
                    found = true;
                    classMethods.push_back(method);
                    break;
                }
            }
            if (!found) return nullptr;
        }

        std::vector<FieldDecl*> classFields;
        for (auto&& ifield: iface->fields) {
            bool found = false;
            for (auto&& field: cls->fields) {
                if (field->name == ifield->name && isAssignableFrom(ifield->declType, field->declType)) {
                    found = true;
                    classFields.push_back(field);
                    break;
                }
            }
            if (!found) return nullptr;
        }

        auto implementation = new Implementation{iface, cls, classMethods, classFields};
        iface->implementations.insert(std::make_pair(cls, implementation));
        return implementation;
    }

    void TypeChecker::printMissingMembers(ClassDecl* cls, InterfaceDecl* iface) {
        std::stringstream sstr;
        for (auto&& imethod: iface->methods) {
            bool found = false;
            for (auto&& method: cls->methods) {
                if (method->name == imethod->name && isCallable(method->declType, imethod->type->paramTypes)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                info(*imethod, imethod->getDescription());
            }
        }

        std::vector<FieldDecl*> classFields;
        for (auto&& ifield: iface->fields) {
            bool found = false;
            for (auto&& field: cls->fields) {
                if (field->name == ifield->name && isAssignableFrom(ifield->declType, field->declType)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                info(*ifield, ifield->getDescription());
            }
        }
    }

    bool isAssignableFrom(TypeDecl* to, TypeDecl* from) {
        if (auto toalias = to->as<TypeAliasDecl>()) {
            to = toalias->typeExpr->typeValue;
        }
        
        if (auto fromalias = from->as<TypeAliasDecl>()) {
            from = fromalias->typeExpr->typeValue;
        }

        if (to->as<PointerType>() && !(from->as<VoidType>() || from->as<InvalidType>())) return true;
        if (to->as<ClassDecl>() && to == from) return true;
        if (to->as<IntType>() && (from->as<IntType>() || from->as<FloatType>())) return true;
        if (to->as<FloatType>() && (from->as<FloatType>() || from->as<IntType>())) return true;
        if (to->as<BoolType>() && from->as<BoolType>()) return true;
        if (to->as<EnumDecl>() && from == to) return true;
        if (auto tounion = to->as<UnionType>()) {
            if (from == to) return true;
            for (auto& type: tounion->containedTypes) {
                if (isAssignableFrom(type, from)) {
                    return true;
                }
            }
            return false;
        }
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
        if (auto fromAlias = from->as<TypeAliasDecl>()) {
            return isAssignableFrom(to, fromAlias->typeExpr->typeValue);
        }
        if (auto toAlias = to->as<TypeAliasDecl>()) {
            return isAssignableFrom(toAlias->typeExpr->typeValue, from);
        }
        if (from == &NullType::instance && to == &NullType::instance) {
            return true;
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

    Expr* addCast(Expr* expr, TypeDecl* targetType) {
        auto fromType = expr->type;
        if (auto fromalias = fromType->as<TypeAliasDecl>()) {
            fromType = fromalias->typeExpr->typeValue;
        }
        if (auto toalias = targetType->as<TypeAliasDecl>()) {
            targetType = toalias->typeExpr->typeValue;
        }

        if (targetType == fromType) {
            return expr;
        }

        auto toIface = targetType->as<InterfaceDecl>();
        auto toUnion = targetType->as<UnionType>();
        auto fromClass = fromType->as<ClassDecl>();
        
        if (fromClass && toIface) {
            auto cast = new CastExpr();
            cast->sourceExpr = expr;
            cast->targetType = targetType;
            cast->implementation = getImplementation(fromClass, toIface);
            cast->type = targetType;
            return cast;
        }

        if (toUnion) {
            if (fromType->_name == "ClassDecl") {
                int i=0;
            }
            for (auto& type: toUnion->containedTypes) {
                if (isAssignableFrom(type, fromType)) {
                    auto cast = new CastExpr();
                    cast->sourceExpr = addCast(expr, type);
                    cast->targetType = targetType;
                    cast->type = targetType;
                    return cast;
                }
            }
        }
        
        auto cast = new CastExpr();
        cast->sourceExpr = expr;
        cast->targetType = targetType;
        cast->type = targetType;
        return cast;
    }

    void prepareArguments(std::vector<Expr*>& arguments, FuncType* ftype) {
        for (size_t i = 0; i < ftype->paramTypes.size(); ++i) {
            arguments[i] = addCast(arguments[i], ftype->paramTypes[i]);
        }
    }


    IntType* getSignedType(IntType* intt) {
        if (intt == &IntType::u8) return &IntType::i8;
        if (intt == &IntType::u16) return &IntType::i16;
        if (intt == &IntType::u32) return &IntType::i32;
        if (intt == &IntType::u64) return &IntType::i64;
        return intt;
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

            if (candidate->declType->paramTypes == argTypes) {
                // exact match
                return {candidate};
            }

            if (isCallable(candidate->declType, argTypes)) {
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

    void TypeChecker::check(ModDecl& n) {
        for (auto& fun: n.functions) {
            check(*fun);
        }
        
        for (auto& cls: n.classes) {
            check(*cls);
        }
    }

    void TypeChecker::visit(IsExpr& n) {
        visitChild(n.target);
        
        auto t = getType(n.target);
        auto alias = t->as<TypeAliasDecl>();
        if (alias) {
			t = alias->typeExpr->typeValue;
        }
		auto un = t->as<UnionType>();
		auto iface = t->as<InterfaceDecl>();
		if (un) {
			auto checkedType = n.typeExpr->typeValue;

			if (!un->containsType(checkedType)) {
				error(n, "'" + checkedType->getFullName() + "' is not part of union type '" + t->getFullName() + "'.");
				return;
			}

			n.typeTag = un->getTypeTag(checkedType);

			if (n.target->node && n.parent->as<IfStmt>()) {
				refinements.back()[n.target->node].push_back(Refinement{ n.target->node, checkedType });
			}
		}
		else if (iface) {
			auto cls = n.typeExpr->typeValue->as<ClassDecl>();
			if (!cls) {
				error(n, "Checked expression must have class type");
				return;
			}
			auto impl = getImplementation(cls, iface);
			if (!impl) {
				error(n, "This condition always evaluates to false because " + cls->getFullName() + " does not implement " + iface->getFullName());
                info("The following members are missing:");
                printMissingMembers(cls, iface);

				return;
			}
			n.implementation = impl;

			if (n.target->node && n.parent->as<IfStmt>()) {
				refinements.back()[n.target->node].push_back(Refinement{ n.target->node, cls });
			}
		}
		else {
            error(n, t->getFullName() + " is neither a union type nor an interface.");
            return;
        }

		n.type = &BoolType::instance;
	}

    void TypeChecker::check(ClassDecl& n) {
        if (!n.genericParams.empty() && n.genericArguments.empty()) {
            // this is a generic class that possibly has instantiations
            for (auto& cls: n.reifiedClasses) {
                check(*cls);
            }            
        }
        else {
            // this is a non-generic class or an instantiation of a generic class
            auto oldclass = _class;
            _class = &n;

            for (auto& method: n.methods) {
                check(*method);
            }

            _class = oldclass;
        }
    }

    void TypeChecker::check(FuncDecl& n) {
        auto oldfunction = function;
        function = &n;

        bool unreachableWarning = false;
        for (auto&& stmt: n.stmts) {
            if (n.returns && !unreachableWarning) {
                unreachableWarning = true;
                warning(*stmt, "Unreachable code detected.");
            }
            visitChild(stmt);
            if (stmt->returns) n.returns = true;
        }

        if (!n.isExternal && !n.returns && n.declType->returnType != &VoidType::instance) {
            error(n, n.name + ": Not all paths return a value.");
        }

        function = oldfunction;
    }
    
    void TypeChecker::visit(MapLitExpr& n) {
        TypeDecl* keyType = nullptr;
        TypeDecl* valueType = nullptr;
        auto exptype = expectedType;
        if (exptype) {
            if (auto alias = exptype->as<TypeAliasDecl>()) {
                exptype = alias->typeExpr->typeValue;
            }
            std::vector<TypeDecl*> typesToCheck{exptype};
            if (auto un = exptype->as<UnionType>()) {
                typesToCheck.assign(un->containedTypes.begin(), un->containedTypes.end());
            }
            for (auto& typeToCheck: typesToCheck) {
                auto constructors = typeToCheck->getMethods("init");
                for (auto& node: constructors) {
                    auto ctor = node->as<FuncDecl>();
                    if (ctor->params.size() == 2) {
                        auto p1 = ctor->params[0]->declType->as<ArrayType>();
                        auto p2 = ctor->params[1]->declType->as<ArrayType>();
                        if (p1 && p2) {
                            keyType = p1->baseType;
                            valueType = p2->baseType;
                            n.keyArrayType = p1;
                            n.valueArrayType = p2;
                            n.constructor = ctor;
                            exptype = typeToCheck;
                            break;
                        }
                    }
                }

                if (n.constructor) {
                    break;
                }
            }
        }
        
        if (!keyType || !valueType) {
            error(n, "Map literal must be assigned to typed variabe");
            return;
        }

        unifyChildren(n.keys, keyType);
        unifyChildren(n.values, valueType);

        n.type = exptype;
    }

    void TypeChecker::visit(ArrayLitExpr& n) {
        if (expectedType) {
            auto exptype = expectedType;
            // find array
            if (auto alias = exptype->as<TypeAliasDecl>()) {
                exptype = alias->typeExpr->typeValue;
            }

            TypeDecl* expectedElementType = nullptr;
            if (auto arr = exptype->as<ArrayType>()) {
                expectedElementType = arr->baseType;
            }
            else {
                std::vector<TypeDecl*> typesToCheck{exptype};
                if (auto un = exptype->as<UnionType>()) {
                    typesToCheck.assign(un->containedTypes.begin(), un->containedTypes.end());
                }
                for (auto& typeToCheck: typesToCheck) {
                    auto constructors = typeToCheck->getMethods("init");
                    for (auto& node: constructors) {
                        auto ctor = node->as<FuncDecl>();
                        if (ctor->params.size() == 1) {
                            auto p1 = ctor->params[0]->declType->as<ArrayType>();
                            if (p1) {
                                exptype = typeToCheck;
                                expectedElementType = p1->baseType;
                                n.type = typeToCheck;
                                n.constructor = ctor;
                                break;
                            }
                        }
                    }

                    if (expectedElementType) {
                        break;
                    }
                }
            }

            if (!expectedElementType) {
                error(n, exptype->getFullName() + " has no constructor that takes an array.");
                return;
            }

            unifyChildren(n.elements, expectedElementType);
        }
        else {
            if (n.elements.empty()) {
                error(n, "Empty array literal is not allowed without typed left hand side.");
                return;
            }

            visitChildren(n.elements);

            auto types = unique(getTypes(n.elements));
            if (types.size() > 1) {
                error(n, "Array literal with untyped left hand side can not contain differently typed elements.");
                return;
            }

            if (types.front() == &InvalidType::instance) {
                error(n, "Invalid array element.");
                return;
            }

            n.type = ArrayType::get(types.front());
        }
    }

    void TypeChecker::visit(SubscriptExpr& n) {
        visitChildren(n.arguments);
        visitChild(n.callTarget);

        auto targetType = getType(n.callTarget);
        if (auto arr = targetType->as<ArrayType>()) {
            if (n.arguments.size() != 1) {
                error(n, "Array subscript operator takes one argument.");
                return;
            }

            if (!isAssignableFrom(&IntType::u64, getType(n.arguments.front()))) {
                error(n, "Array subscript operator argument must be integer.");
                return;
            }

            n.type = arr->baseType;
            n.context = addCast(n.callTarget, targetType);
            n.arrayIndex = addCast(n.arguments.front(), &IntType::u64);
        }
        else {
            auto candidates = extract<FuncDecl>(targetType->getMethods("[]"));
            auto funs = findOverload(candidates, n.arguments);
            if (funs.empty()) {
                error(n, "Type '" + targetType->getFullName() + "' has no subscript operator with arguments (" + getTypes(n.arguments) + ").");
                return;
            }
            
            if (funs.size() > 1) {
                error(n, "Type '" + targetType->getFullName() + "' has multiple matching subscript operators for arguments (" + getTypes(n.arguments) + ").");
                return;
            }

            auto fun = funs.front();
            n.type = fun->declType->returnType;
            n.subscriptFunction = fun;
            n.callTarget = addCast(n.callTarget, targetType);

            prepareArguments(n.arguments, fun->declType);
        }
    }

    void TypeChecker::visit(VarDecl& n) {
        if (n.initializer) {
            unify(n.initializer, n.typeExpr ? n.declType : nullptr);
            if (n.initializer->type == &InvalidType::instance) {
                return;
            }

            if (n.typeExpr == nullptr) {
                n.declType = n.initializer->type;
            }
            
            if (!isAssignableFrom(n.declType, n.initializer->type)) {
                error(n, n.name + ": Can not assign '" + n.initializer->type->getFullName() + "' to '" + n.declType->getFullName() + "'.");
                return;
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
            if (!isCallable(ftype, argTypes)) {
                error(*n.callTarget, "'" + n.callTarget->type->getFullName() + "' is not callable with arguments (" + argTypes + ")");
                return;
            }

            n.type = ftype->returnType;
            for (int i = 0; i < n.arguments.size(); ++i) {
                n.arguments[i] = addCast(n.arguments[i], argTypes[i]);
            }
            prepareArguments(n.arguments, ftype);
        }
        else if (n.callTarget->candidates.size() > 0) {
            // overloaded function call
            auto funs = findOverload(n.callTarget, argTypes);
            if (funs.empty()) {
                error(n, "'" + n.callTarget->candidates.front()->name + "': No matching overload found. arguments are: (" + argTypes + ")");
                return;
            }
            
            if (funs.size() > 1) {
                error(n, "Multiple matching overloads found.");
                return;
            }

            auto fun = funs.front();
            n.callTarget->type = fun->declType;
            n.callTarget->node = fun;
            n.callTarget->candidates.clear();
            n.type = fun->declType->returnType;
            for (int i = 0; i < n.arguments.size(); ++i) {
                n.arguments[i] = addCast(n.arguments[i], argTypes[i]);
            }
            prepareArguments(n.arguments, fun->declType);
        }
        else {
            error(n, "Can not call value of type '" + n.callTarget->type->getFullName() + "'");
            return;
        }
    }

    void TypeChecker::visit(RetStmt& n) {
        n.returns = true;

        if (function->declType->returnType == &VoidType::instance) {
            return;
        }

        if (!n.expression) {
            error(n, "non-void function must return a value.");
            return;
        }

        visitChild(n.expression);

        if (!isAssignableFrom(function->declType->returnType, n.expression->type)) {
            error(n, function->name + ": Incompatible return type. Returning '" + n.expression->type->getFullName() + "' from '" + function->declType->returnType->getFullName() + "' function.");
            return;
        }

        if (n.expression->type == &NullType::instance) {
            int i =0 ;
        }

        n.expression = addCast(n.expression, function->declType->returnType);
    }

    void TypeChecker::visit(ExprStmt& n) {
        visitChild(n.expression);
    }

    void TypeChecker::visit(LitExpr& n) {
        switch (n.token.type) {
            case TokenType::Integer: {
                n.type = &IntType::i64;
                break;
            }

            case TokenType::Float: {
                n.type = &FloatType::f64;
                break;
            }

            case TokenType::Boolean:
                n.type = &BoolType::instance;
                break;

            case TokenType::String:
                n.type = ClassDecl::String;
                break;

            case TokenType::Null:
                n.type = &NullType::instance;
                break;

            default:
                error(n, "Wrong token type for literal expression.");
                return;
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

        auto ltype = getType(n.left);
        auto rtype = getType(n.right);

        auto lint = ltype->as<IntType>();
        auto rint = rtype->as<IntType>();
        
        auto lfloat = ltype->as<FloatType>();
        auto rfloat = rtype->as<FloatType>();

        bool leftScalar = lint || lfloat;
        bool rightScalar = rint || rfloat;

        auto op = n.op;

        auto methods = ltype->getMethods(getTokenVal(op));
        if (!methods.empty()) {
            auto ol = findOverload(extract<FuncDecl>(methods), std::vector<TypeDecl*>{rtype});
            if (ol.size() == 1) {
                n.function = ol.front();
                n.type = n.function->declType->returnType;
                n.right = addCast(n.right, n.function->declType->paramTypes.front());
                n.left = addCast(n.left, ltype);
                return;
            }
        }

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
            
            if (
                !(
                    (leftScalar && rightScalar) ||
                    (op == TokenType::Plus && ltype == ClassDecl::String && rtype == ClassDecl::String) ||
                    (op == TokenType::Plus && ltype == ClassDecl::String && rtype->as<IntType>())
                )
            ) {
                error(n, "Binary operator '" + getTokenName(op) + "' is only applicable to scalar values. Types are '" + ltype->getFullName() + "' and '" + rtype->getFullName() + "'.");
            }

            if (op == TokenType::LessThan || op == TokenType::GreaterThan || op == TokenType::LessThanEquals || op == TokenType::GreaterThanEquals) {
                n.type = &BoolType::instance;
                n.right = addCast(n.right, n.left->type);
            }
            else {
                if (lint && rint) {
                    n.type = lint;
                    n.right = addCast(n.right, lint);
                }
                else if (lfloat && rfloat) {
                    n.type = lfloat;
                    n.right = addCast(n.right, lfloat);
                }
                else if (lfloat) {
                    n.type = lfloat;
                    n.right = addCast(n.right, lfloat);
                }
                else if (rfloat) {
                    n.type = rfloat;
                    n.left = addCast(n.left, rfloat);
                }
                else if (ltype == ClassDecl::String) {
                    n.type = ClassDecl::String;
                }
            }
        }
        else if (op == TokenType::Percent) {
            if (!(lint && rint)) {
                error(n, "Binary operator '" + getTokenName(op) + "' is only applicable to integer values. Types are '" + ltype->getFullName() + "' and '" + rtype->getFullName() + "'.");
            }
            n.type = lint;
            n.right = addCast(n.right, lint);
        }
        else if (op == TokenType::EqualsEquals || op == TokenType::ExclamationMarkEquals) {
            if (!((leftScalar && rightScalar) || (ltype == rtype))) {
                error(n, "Binary operator '" + getTokenName(op) + "' is not applicable to types '" + ltype->getFullName() + "' and '" + rtype->getFullName() + "'.");
            }
            n.type = &BoolType::instance;
        }
        else if (op == TokenType::PipePipe || op == TokenType::AmpAmp) {
            if (!(ltype == &BoolType::instance && rtype == &BoolType::instance)) {
                error(n, "Binary operator '" + getTokenName(op) + "': Both operands must have boolean type. Actual types are '" + ltype->getFullName() + "' and '" + rtype->getFullName() + "'.");
            }
            n.type = &BoolType::instance;
        }
        else {
            error(n, "Invalid binary operator '" + getTokenName(op) + "'");
        }
        int i = 0;
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
                default: error(n, "Assignment expression: invalid op");
            }
            auto binop = new BinopExpr();
            binop->op = op;
            binop->left = n.left;
            binop->right = n.right;
            
            n.right = binop;
        }

        unify(n.right, getType(n.left));

        auto ltype = getType(n.left);
        auto rtype = getType(n.right);

        if (!isAssignableFrom(ltype, rtype)) {
            error(n, "Can not assign " + rtype->getFullName() + " to " + ltype->getFullName());
        }

        if (n.left->node) {
            if (auto var = n.left->node->as<VarDecl>()) {
                n.type = var->declType;
            }
            else if (auto param = n.left->node->as<Param>()) {
                n.type = param->typeExpr->typeValue;
            }
            else if (auto field = n.left->node->as<FieldDecl>()) {
                n.type = field->typeExpr->typeValue;
            }
            else if (auto ifd = n.left->node->as<InterfaceFieldDecl>()) {
                n.type = ifd->typeExpr->typeValue;
            }
            else {
                error(n, "Assignment target must be storage node.");
                return;
            }
        }
        else if (n.left->arrayIndex) {
            n.type = ltype;
        }
        else {
            error(n, std::string("Can not assign to ") + n.left->getTypeInfo()->getName());
            return;
        }
        
        n.right = addCast(n.right, ltype);
    }

    void TypeChecker::visit(IdExpr& n) {
        if (n.node) {
            if (auto var = n.node->as<VarDecl>()) {
                n.type = var->declType; // getType(var);
            }
            else if (auto param = n.node->as<Param>()) {
                n.type = param->declType; // getType(param);
            }
            else if (auto fun = n.node->as<FuncDecl>()) {
                n.type = fun->declType;
            }
        }
    }

    void TypeChecker::visit(ScopeExpr& n) {
        visitChild(n.scopeTarget);
        auto type = getType(n.scopeTarget);

        if (n.name == "has") {
            int i=0;
        }

        if (auto typetype = type->as<TypeType>()) {
            if (auto member = n.scopeTarget->typeValue->getMember(n.name)) {
                n.node = member;
                if (auto td = n.node->as<TypeDecl>()) {
                    n.type = &TypeType::instance;
                    n.typeValue = td;
                }
                else if (auto fd = n.node->as<FuncDecl>()) {
                    n.type = fd->declType;
                }
                else if (auto en = n.node->as<EnumElement>()) {
                    n.type = n.scopeTarget->typeValue;
                }
                else {
                    error(n, "unhandled member kind");
                }
            }
            else {
                error(n, "Type '" + typetype->getFullName() + "' has no member named '" + n.name + "'");
            }
        }
        else if (type) {
            n.scopeTarget = addCast(n.scopeTarget, type);
            n.context = n.scopeTarget;
            auto methods = type->getMethods(n.name);
            if (methods.size() == 1) {
                n.node = methods.front();
                if (auto iface = methods.front()->as<InterfaceMethodDecl>()) {
                    n.type = iface->type;
                }
                else {
                    n.type = methods.front()->as<FuncDecl>()->declType;
                }
            }
            else if (methods.size() > 1) {
                n.candidates = extract<FuncDecl>(methods);
                n.type = &OverloadedFuncType::instance;
            }
            else {
                n.node = type->getMember(n.name);
                if (n.node) {
                    if (auto field = n.node->as<FieldDecl>()) {
                        n.type = field->declType;
                    }
                    else if (auto fun = n.node->as<FuncDecl>()) {
                        n.type = fun->declType;
                    }
                    else if (auto met = n.node->as<InterfaceMethodDecl>()) {
                        n.type = met->type;
                    }
                    else if (auto ifd = n.node->as<InterfaceFieldDecl>()) {
                        n.type = ifd->declType;
                    }
                    else {
                        error(n, "unhandled member kind");
                    }
                }
                else {
                    error(n, "Type '" + type->getFullName() + "' has no member named '" + n.name + "'");
                }
            }
        }        
        else {
            error(n, "Scope operator not applicable to type '" + n.scopeTarget->type->getFullName() + "'.");
        }
    }

    void TypeChecker::visit(IfStmt& n) {
        refinements.push_back({});
        visitChild(n.condition);
        if (getType(n.condition) != &BoolType::instance) {
            error(*n.condition, "Condition must yield boolean value. Is " + n.condition->type->getFullName());
        }
        n.condition = addCast(n.condition, &BoolType::instance);
        visitChild(n.trueBranch);
        auto ret = n.trueBranch->returns;
        if (n.falseBranch) {
            negateRefinements();
            visitChild(n.falseBranch);
            n.returns = ret && n.falseBranch->returns;
        }
        refinements.pop_back();
    }

    void TypeChecker::negateRefinements() {
        std::map<Node*, std::vector<Refinement>> newRefinements;
        auto refs = refinements.back();
        refinements.pop_back();
        for (auto& pair: refs) {
            auto& node = pair.first;
            for (auto&& ref: pair.second) {
                if (ref.type) {
                    if (auto var = ref.node->as<VarDecl>()) {
                        auto t = getType(var);
                        auto uni = t->as<UnionType>();
                        if (!uni && t->as<TypeAliasDecl>()) {
                            uni = t->as<TypeAliasDecl>()->typeExpr->typeValue->as<UnionType>();
                        }
                        if (uni) {
                            if (uni->containedTypes.size() == 2) {
                                newRefinements[node].push_back(Refinement{node, uni->getComplementaryType(ref.type)});
                            }
                        }
                    }
                    else if (auto par = ref.node->as<Param>()) {
                        auto t = getType(par);
                        auto uni = t->as<UnionType>();
                        if (!uni && t->as<TypeAliasDecl>()) {
                            uni = t->as<TypeAliasDecl>()->typeExpr->typeValue->as<UnionType>();
                        }
                        if (uni) {
                            if (uni->containedTypes.size() == 2) {
                                newRefinements[node].push_back(Refinement{node, uni->getComplementaryType(ref.type)});
                            }
                        }
                    }
                }
            }
        }
        refinements.push_back(newRefinements);
    }

    void TypeChecker::visit(NewExpr& n) {
        visitChild(n.typeExpr);
        visitChildren(n.arguments);

        auto type = n.typeExpr->typeValue;
        if (auto alias = type->as<TypeAliasDecl>()) {
            type = alias->typeExpr->typeValue;
        }

        if (auto cls = type->as<ClassDecl>()) {
            n.type = cls;

            auto inits = extract<FuncDecl>(cls->getMethods("init"));
            if (inits.empty()) {
                if (!n.arguments.empty()) {
                    error(n, "'" + type->getFullName() + "' has no matching constructor for arguments (" + getTypes(n.arguments) + ").");
                }
            }
            else {
                auto init = findOverload(inits, n.arguments);
                if (init.size() == 1) {
                    n.initMethod = init.front();
                    prepareArguments(n.arguments, n.initMethod->declType);
                }
                else if (init.empty()) {
                    error(n, "No matching constructors found for arguments (" + getTypes(n.arguments) + ").");
                }
                else {
                    error(n, "Multiple matching constructors found for arguments (" + getTypes(n.arguments) + ").");
                }
            }
        }
        else if (auto arr = type->as<ArrayType>()) {
            if (n.arguments.size() == 1) {
                if (isAssignableFrom(&IntType::u64, getType(n.arguments.front()))) {
                    n.type = arr;
                }
                else {
                    error(n, "Array constructor argument must have unsigned int type.");
                }
            }
            else {
                error(n, "Array constructor requires exactly one argument of type unsigned int.");
            }
        }
        else {
            error(n, "Only class and array types are instantiable, type is '" + type->getFullName() + "'");
        }
    }

    void TypeChecker::visit(WhileStmt& n) {
        visitChild(n.condition);
        if (getType(n.condition) != &BoolType::instance) {
            error(*n.condition, "Condition must yield boolean value. Is " + n.condition->type->getFullName());
        }

        visitChild(n.body);
    }

    void TypeChecker::visit(CastExpr& n) {
        visitChild(n.sourceExpr);
        n.targetType = n.targetTypeExpr->typeValue;
        n.type = n.targetType;

        if (!isAssignableFrom(n.targetType, n.sourceExpr->type)) {
            error(n, "Can not assign '" + n.sourceExpr->type->getFullName() + "' to '" + n.targetType->getFullName() + "'.");
        }
    }

    void TypeChecker::visit(PostfixExpr& n) {
        visitChild(n.target);
        if (!n.target->node || !(n.target->node->as<FieldDecl>() || n.target->node->as<Param>() || n.target->node->as<VarDecl>())) {
            error(n, "Target for operator '" + getTokenName(n.op) + "' must be a reference to a mutable value.");
            return;
        }
        if (!(n.target->type->as<FloatType>() || n.target->type->as<IntType>())) {
            error(n, "Operator '" + getTokenName(n.op) + "' is only applicable to scalar values. Target type is '" + n.target->type->getFullName() + "'.");
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

            default:
            error(n, "Unhandled unary prefix operator '" + getTokenName(n.op) + "'.");
        }
    }

    void TypeChecker::visit(ThisExpr& n) {
        if (_class) {
            n._class = _class;
            n.type = _class;
        }
        else {
            error(n, "Found this-expression outside of class.");
        }
    }
}