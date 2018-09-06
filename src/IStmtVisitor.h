// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_IStmtVisitor_h
#define Strela_IStmtVisitor_h

namespace Strela {
    template<typename T> class IStmtVisitor {
    public:
        virtual T visit(class ModDecl&) = 0;
        virtual T visit(class ImportStmt&) = 0;
        virtual T visit(class FuncDecl&) = 0;
        virtual T visit(class Param&) = 0;
        virtual T visit(class VarDecl&) = 0;
        virtual T visit(class ClassDecl&) = 0;
        virtual T visit(class FieldDecl&) = 0;
        virtual T visit(class EnumDecl&) = 0;
        virtual T visit(class EnumElement&) = 0;
        virtual T visit(class InterfaceDecl&) = 0;
        virtual T visit(class InterfaceMethodDecl&) = 0;
        virtual T visit(class GenericParam&) = 0;
        virtual T visit(class TypeAliasDecl&) = 0;
        virtual T visit(class RetStmt&) = 0;
        virtual T visit(class BlockStmt&) = 0;
        virtual T visit(class ExprStmt&) = 0;
        virtual T visit(class IfStmt&) = 0;
        virtual T visit(class WhileStmt&) = 0;
        virtual T visit(class ArrayType&) {}
        virtual T visit(class FloatType&) {}
        virtual T visit(class FuncType&) {}
        virtual T visit(class IntType&) {}
        virtual T visit(class NullType&) {}
        virtual T visit(class VoidType&) {}
        virtual T visit(class BoolType&) {}
        virtual T visit(class TypeType&) {}
        virtual T visit(class UnionType&) {}
        virtual T visit(class InvalidType&) {}
        virtual T visit(class PointerType&) {}
        virtual T visit(class OverloadedFuncType&) {}
    };
}

#endif