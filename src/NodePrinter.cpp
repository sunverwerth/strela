#include "NodePrinter.h"
#include "Ast/nodes.h"

#include <iostream>

namespace Strela {
    void NodePrinter::visit(AstModDecl& n) {
        std::cout << "module " << n.name << " {\n";
        push();
        for (auto&& en: n.enums) {
            std::cout << indent;
            visitChild(en);
        }
        for (auto&& cls: n.classes) {
            std::cout << indent;
            visitChild(cls);
        }
        for (auto&& fun: n.functions) {
            std::cout << indent;
            visitChild(fun);
        }
        pop();
        std::cout << "}\n";
    }

    void NodePrinter::visit(AstClassDecl& n) {
        if (n.isExported) {
            std::cout << "export ";
        }
        std::cout << "class " << n.name << " {\n";
        push();
        for (auto&& field: n.fields) {
            std::cout << indent;
            visitChild(field);
        }
        for (auto&& fun: n.methods) {
            std::cout << indent;
            visitChild(fun);
        }
        pop();
        std::cout << indent << "}\n";
    }

    void NodePrinter::visit(AstFuncDecl& n) {
        if (n.isExported) {
            std::cout << "export ";
        }
        std::cout << "function " << n.name << "(";
        for(auto&& param: n.params) {
            visitChild(param);
            if (&param != &n.params.back()) {
                std::cout << ", ";
            }
        }
        std::cout << "): ";
        if (n.returnTypeExpr) {
            visitChild(n.returnTypeExpr);
        }
        else {
            std::cout << "void";
        }
        std::cout << " {\n";
        push();
        for (auto&& stmt: n.stmts) {
            std::cout << indent;
            visitChild(stmt);
        }
        pop();
        std::cout << indent << "}\n";
    }

    void NodePrinter::visit(AstBlockStmt& n) {
        std::cout << "{\n";
        push();
        for (auto&& stmt: n.stmts) {
            std::cout << indent;
            visitChild(stmt);
        }
        pop();
        std::cout << indent << "}\n";
    }

    void NodePrinter::visit(AstParam& n) {
        std::cout << n.name << ": ";
        visitChild(n.typeExpr);
    }

    void NodePrinter::visit(AstVarDecl& n) {
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

    void NodePrinter::visit(AstIdExpr& n) {
        std::cout << n.name;
    }

    void NodePrinter::visit(AstExprStmt& n) {
        visitChild(n.expression);
        std::cout << ";\n";
    }

    void NodePrinter::visit(AstCallExpr& n) {
        visitChild(n.callTarget);
        std::cout << "(";
        for (auto&& arg: n.arguments) {
            arg->accept(*this);
            if (&arg != &n.arguments.back()) {
                std::cout << ", ";
            }
        }
        std::cout << ")";
    }

    void NodePrinter::visit(AstRetStmt& n) {
        std::cout << "return ";
        if (n.expression) {
            visitChild(n.expression);
        }
        std::cout << ";\n";

    }

    std::string escape(const std::string&);

    void NodePrinter::visit(AstLitExpr& n) {
        if (n.token.type == TokenType::String) {
            std::cout << "\"" << escape(n.token.value) << "\"";
        }
        else {
            std::cout << n.token.value;
        }
    }

    void NodePrinter::visit(AstBinopExpr& n) {
        visitChild(n.left);
        std::cout << " " << n.startToken.value << " ";
        visitChild(n.right);
    }

    void NodePrinter::visit(AstScopeExpr& n) {
        visitChild(n.scopeTarget);
        std::cout << "." << n.name;
    }

    void NodePrinter::visit(AstIfStmt& n) {
        std::cout << "if (";
        visitChild(n.condition);
        std::cout << ") ";
        visitChild(n.trueBranch);
        if (n.falseBranch) {
            std::cout << "else ";
            visitChild(n.falseBranch);
        }
    }

    void NodePrinter::visit(AstFieldDecl& n) {
        std::cout << "var " << n.name << ": ";
        visitChild(n.typeExpr);
        std::cout << ";\n";
    }

    void NodePrinter::visit(AstNewExpr& n) {
        std::cout << "new ";
        visitChild(n.typeExpr);
    }

    void NodePrinter::visit(AstAssignExpr& n) {
        visitChild(n.left);
        std::cout << " = ";
        visitChild(n.right);
    }

    void NodePrinter::visit(AstIdTypeExpr& n) {
        std::cout << n.name;
    }

    void NodePrinter::visit(AstWhileStmt& n) {
        std::cout << "while (";
        visitChild(n.condition);
        std::cout << ") ";
        visitChild(n.body);
    }

    void NodePrinter::visit(AstPostfixExpr& n) {
        visitChild(n.target);
        std::cout << n.startToken.value;
    }

    void NodePrinter::visit(AstArrayTypeExpr& n) {
        visitChild(n.base);
        std::cout << "[]";
    }

    void NodePrinter::visit(AstImportStmt& n) {
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

    void NodePrinter::visit(AstUnaryExpr& n) {
        std::cout << n.startToken.value;
        visitChild(n.target);
    }

    void NodePrinter::visit(AstEnumDecl& n) {
        std::cout << "enum {\n";
        push();
        for (auto&& el: n.elements) {
            std::cout << indent;
            visitChild(el);
        }
        pop();
        std::cout << indent <<  "}\n";
    }

    void NodePrinter::visit(AstEnumElement& n) {
        std::cout << n.name << ",\n";
    }

    void NodePrinter::push() {
        ++indentation;
        indent = std::string(indentation * 4, ' ');
    }

    void NodePrinter::pop() {
        --indentation;
        indent = std::string(indentation * 4, ' ');
    }

}