// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_IStmtVisitor_h
#define Strela_IStmtVisitor_h

namespace Strela {
    class IStmtVisitor {
    public:
        virtual void visit(class ModDecl&) = 0;
        virtual void visit(class ImportStmt&) = 0;
        virtual void visit(class FuncDecl&) = 0;
        virtual void visit(class Param&) = 0;
        virtual void visit(class VarDecl&) = 0;
        virtual void visit(class ClassDecl&) = 0;
        virtual void visit(class FieldDecl&) = 0;
        virtual void visit(class EnumDecl&) = 0;
        virtual void visit(class EnumElement&) = 0;
        virtual void visit(class InterfaceDecl&) = 0;
        virtual void visit(class InterfaceMethodDecl&) = 0;
        virtual void visit(class GenericParam&) = 0;

        virtual void visit(class RetStmt&) = 0;
        virtual void visit(class BlockStmt&) = 0;
        virtual void visit(class ExprStmt&) = 0;
        virtual void visit(class IfStmt&) = 0;
        virtual void visit(class WhileStmt&) = 0;

        // builtin types
        virtual void visit(class ArrayType&) {}
        virtual void visit(class FloatType&) {}
        virtual void visit(class FuncType&) {}
        virtual void visit(class IntType&) {}
        virtual void visit(class NullType&) {}
        virtual void visit(class VoidType&) {}
        virtual void visit(class BoolType&) {}
        virtual void visit(class TypeType&) {}
        virtual void visit(class UnionType&) {}
        virtual void visit(class InvalidType&) {}
        virtual void visit(class PointerType&) {}
        virtual void visit(class OverloadedFuncType&) {}
    };
}

#endif