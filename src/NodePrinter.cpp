// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "NodePrinter.h"
#include "Ast/nodes.h"

#include <iostream>

namespace Strela {
    void NodePrinter::print(ModDecl& n) {
        std::cout << "module " << n._name << " {\n";
        push();
        for (auto&& en: n.enums) {
            indent();
            print(*en);
        }
        for (auto&& cls: n.classes) {
            indent();
            print(*cls);
        }
        for (auto&& ta: n.typeAliases) {
            indent();
            print(*ta);
        }
        for (auto&& fun: n.functions) {
            indent();
            print(*fun);
        }
        pop();
        std::cout << "}\n";
    }

    void NodePrinter::print(ClassDecl& n) {
        std::cout << "\n";
        indent();
        if (n.isExported) {
            std::cout << "export ";
        }
        std::cout << "class " << n._name;
        if (!n.genericParams.empty()) {
            std::cout << "<";
            for (auto&& child: n.genericParams) {
                print(*child);
                if (&child != &n.genericParams.back()) {
                    std::cout << ", ";
                }
            }
            std::cout << ">";
        }
        std::cout << " {\n";
        push();
        for (auto&& field: n.fields) {
            indent();
            print(*field);
        }
        for (auto&& fun: n.methods) {
            indent();
            print(*fun);
        }
        pop();
        indent();
        std::cout << "}\n";
    }

    void NodePrinter::print(FuncDecl& n) {
        std::cout << "\n";
        indent();
        if (n.isExported) {
            std::cout << "export ";
        }
        std::cout << "function " << n.name << "(";
        for (auto&& part: n.params) {
            std::cout << part;
            if (&part != &n.params.back()) {
                std::cout << ", ";
            }
        }
        std::cout << "): ";
        visitChild(n.returnTypeExpr);
        std::cout << " {\n";
        push();
        for (auto&& stmt: n.stmts) {
            indent();
            visitChild(stmt);
        }
        pop();
        indent();
        std::cout << "}\n";
    }

    void NodePrinter::visit(BlockStmt& n) {
        std::cout << "{\n";
        push();
        for (auto&& stmt: n.stmts) {
            indent();
            visitChild(stmt);
        }
        pop();
        indent();
        std::cout << "}\n";
    }

    void NodePrinter::print(Param& n) {
        std::cout << n.name << ": ";
        visitChild(n.typeExpr);
    }

    void NodePrinter::visit(VarDecl& n) {
        std::cout << "var " << n.name;
        if (n.typeExpr) {
            std::cout << ": ";
            visitChild(n.typeExpr);
        }
        if (n.initializer) {
            std::cout << " = ";
            visitChild(n.initializer);
        }
        std::cout << ";\n";
    }

    void NodePrinter::visit(IdExpr& n) {
        std::cout << n.name;
    }

    void NodePrinter::visit(ExprStmt& n) {
        visitChild(n.expression);
        std::cout << ";\n";
    }

    void NodePrinter::visit(CallExpr& n) {
        visitChild(n.callTarget);
        std::cout << "(";
        list(n.arguments, ", ");
        std::cout << ")";
    }

    void NodePrinter::visit(RetStmt& n) {
        std::cout << "return ";
        if (n.expression) {
            visitChild(n.expression);
        }
        std::cout << ";\n";

    }

    std::string escape(const std::string&);

    void NodePrinter::visit(LitExpr& n) {
        if (n.token.type == TokenType::String) {
            std::cout << "\"" << escape(n.token.value) << "\"";
        }
        else {
            std::cout << n.token.value;
        }
    }

    void NodePrinter::visit(BinopExpr& n) {
        std::cout << "(";
        visitChild(n.left);
        std::cout << " " << getTokenVal(n.op) << " ";
        visitChild(n.right);
        std::cout << ")";
    }

    void NodePrinter::visit(ScopeExpr& n) {
        visitChild(n.scopeTarget);
        std::cout << "." << n.name;
    }

    void NodePrinter::visit(IfStmt& n) {
        std::cout << "if (";
        visitChild(n.condition);
        std::cout << ") ";
        visitChild(n.trueBranch);
        if (n.falseBranch) {
            indent();
            std::cout << "else ";
            visitChild(n.falseBranch);
        }
    }

    void NodePrinter::print(FieldDecl& n) {
        std::cout << "var " << n.name << ": ";
        visitChild(n.typeExpr);
        std::cout << ";\n";
    }

    void NodePrinter::visit(NewExpr& n) {
        std::cout << "new ";
        visitChild(n.typeExpr);
        if (!n.arguments.empty()) {
            std::cout << "(";
            list(n.arguments, ", ");
            std::cout << ")";
        }
    }

    void NodePrinter::visit(AssignExpr& n) {
        visitChild(n.left);
        std::cout << " = ";
        visitChild(n.right);
    }

    void NodePrinter::visit(WhileStmt& n) {
        std::cout << "while (";
        visitChild(n.condition);
        std::cout << ") ";
        visitChild(n.body);
    }

    void NodePrinter::visit(PostfixExpr& n) {
        visitChild(n.target);
        std::cout << getTokenVal(n.op);
    }

    void NodePrinter::visit(ArrayTypeExpr& n) {
        visitChild(n.baseTypeExpr);
        std::cout << "[]";
    }

    void NodePrinter::print(ImportStmt& n) {
        std::cout << "import ";
        for (auto&& part: n.parts) {
            std::cout << part;
            if (&part != &n.parts.back()) {
                std::cout << ".";
            }
        }
        if (n.all) std::cout << ".*";
        std::cout << ";\n";
    }

    void NodePrinter::visit(UnaryExpr& n) {
        std::cout << getTokenVal(n.op);
        visitChild(n.target);
    }

    void NodePrinter::print(EnumDecl& n) {
        std::cout << (n.isExported ? "export " : "") << "enum {\n";
        push();
        for (auto&& el: n.elements) {
            indent();
            print(*el);
        }
        pop();
        indent();
        std::cout <<  "}\n";
    }

    void NodePrinter::print(InterfaceDecl& n) {
        std::cout << (n.isExported ? "export " : "") << "interface {\n";
        push();
        for (auto&& m: n.methods) {
            indent();
            print(*m);
        }
        pop();
        indent();
        std::cout << "}\n";
    }

    void NodePrinter::print(InterfaceMethodDecl& n) {
        std::cout << "function " << n.name << "(";
        for (auto&& part: n.params) {
            std::cout << part;
            if (&part != &n.params.back()) {
                std::cout << ", ";
            }
        }
        std::cout << "): ";
        visitChild(n.returnTypeExpr);
        std::cout << ";\n";
    }

    void NodePrinter::print(EnumElement& n) {
        std::cout << n.name << ",\n";
    }

    void NodePrinter::visit(ThisExpr& n) {
        std::cout << "this";
    }

    void NodePrinter::visit(IsExpr& n) {
        visitChild(n.target);
        std::cout << " is ";
        visitChild(n.typeExpr);
    }

    void NodePrinter::visit(CastExpr& n) {
        std::cout << "(";
        visitChild(n.sourceExpr);
        std::cout << "): ";
        visitChild(n.targetTypeExpr);
    }

    void NodePrinter::visit(UnionTypeExpr& n) {
        visitChild(n.base);
        std::cout << " | ";
        visitChild(n.next);
    }
    
    void NodePrinter::visit(ArrayLitExpr& n) {
        std::cout << "[";
        list(n.elements, ", ");
        std::cout << "]";
    }

    void NodePrinter::visit(MapLitExpr& n) {
        std::cout << "{\n";
        push();
        for (size_t i = 0; i < n.keys.size(); ++i) {
            indent();
            n.keys[i]->accept(*this);
            std::cout << ": ";
            n.values[i]->accept(*this);
            std::cout << ",\n";
        }
        pop();
        std::cout << "}";
    }

    void NodePrinter::visit(SubscriptExpr& n) {
        std::cout << "[";
        list(n.arguments, ", ");
        std::cout << "]";
    }

    void NodePrinter::visit(NullableTypeExpr& n) {
        visitChild(n.baseTypeExpr);
        std::cout << "?";
    }

    void NodePrinter::print(GenericParam& n) {
        std::cout << n._name;
    }

    void NodePrinter::visit(GenericReificationExpr& n) {
        visitChild(n.baseTypeExpr);
        std::cout << "<";
        list(n.genericArguments, ", ");
        std::cout << ">";
    }

    void NodePrinter::print(TypeAliasDecl& n) {
        std::cout << "type " << n._name << " = ";
        visitChild(n.typeExpr);
        std::cout << ";\n";
    }

    void NodePrinter::indent() {
        std::cout << indentstring;
    }

    void NodePrinter::push() {
        ++indentation;
        indentstring = std::string(indentation * 4, ' ');
    }

    void NodePrinter::pop() {
        --indentation;
        indentstring = std::string(indentation * 4, ' ');
    }

}