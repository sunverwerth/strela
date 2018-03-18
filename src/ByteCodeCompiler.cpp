#include "ByteCodeCompiler.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "ByteCodeChunk.h"
#include "Opcode.h"
#include "Scope.h"

#include <sstream>
#include <cstring>

namespace Strela {
    template<typename T> uint64_t pack(const T& v) {
		uint64_t ret{ 0 };
		memcpy(&ret, &v, sizeof(T));
		return ret;
    }

    ByteCodeCompiler::ByteCodeCompiler(ByteCodeChunk& chunk): chunk(chunk) {
    }

    void ByteCodeCompiler::visit(ModDecl& n) {
        visitChildren(n.functions);
        visitChildren(n.classes);

        // fixup function pointers
        for (auto&& fixup: functionFixups) {
            if (fixup.second->opcodeStart == 0xdeadbeef) {
                fixup.second->accept(*this);
            }
            auto index = chunk.addConstant(VMValue(int64_t(fixup.second->opcodeStart)));
            chunk.writeArgument(fixup.first, index);
        }

        auto mainSymbol = n.getMember("main");
        if (!mainSymbol) {
            error(n, "No entry point");
        }
        auto mainFunc = mainSymbol->as<FuncDecl>();
        if (!mainFunc) {
            error(n, "Entry point main must be a function.");
        }

        chunk.main = mainFunc->opcodeStart;
    }

    void ByteCodeCompiler::visit(ClassDecl& n) {
        visitChildren(n.methods);
    }

    void ByteCodeCompiler::visit(FuncDecl& n) {
        auto oldfunc = function;
        function = &n;
        n.opcodeStart = chunk.opcodes.size();
        std::stringstream sstr;
        sstr << n.name << n.type->name;
        chunk.addFunction(n.opcodeStart, sstr.str());
        visitChildren(n.params);
        if (n.numVariables > 0) {
            chunk.addOp(Opcode::GrowStack, n.numVariables);
        }
        visitChildren(n.stmts);

        if (!n.returns) {
            chunk.addOp(Opcode::ReturnVoid, n.params.size());
        }

        function = oldfunc;
    }

    void ByteCodeCompiler::visit(Param&) {
    }

    void ByteCodeCompiler::visit(VarDecl& n) {
        if (n.initializer) {
            n.initializer->accept(*this);
            chunk.addOp(Opcode::StoreVar, n.index);
        }
    }

    void ByteCodeCompiler::visit(IdExpr& n) {
        if (auto fun = n.node->as<FuncDecl>()) {
            auto index = chunk.addOp(Opcode::Const, 255);
            functionFixups[index] = fun;
        }
        else if (auto param = n.node->as<Param>()) {
            chunk.addOp(Opcode::Param, param->index);
        }
        else if (auto var = n.node->as<VarDecl>()) {
            chunk.addOp(Opcode::Var, var->index);
        }
    }

    void ByteCodeCompiler::visit(ExprStmt& n) {
        visitChild(n.expression);
        if (n.expression->type != &VoidType::instance) {
            chunk.addOp(Opcode::Pop);
        }
    }

    void ByteCodeCompiler::visit(CallExpr& n) {
        for (size_t i = n.arguments.size(); i > 0; --i) {
            n.arguments[i - 1]->accept(*this);
        }

        if (n.callTarget->node && n.callTarget->node->as<FuncDecl>()->name == "print") {
            chunk.addOp(Opcode::Print);
        }
        else {
            visitChild(n.callTarget);
            chunk.addOp(Opcode::Call);
        }
    }

    void ByteCodeCompiler::visit(RetStmt& n) {
        if (n.expression) {
            visitChild(n.expression);
            chunk.addOp(Opcode::Return, function->params.size());
        }
        else {
            chunk.addOp(Opcode::ReturnVoid, function->params.size());
        }
    }

    void ByteCodeCompiler::visit(LitExpr& n) {
        int index = 0;
        if (n.type == &IntType::instance) {
            chunk.addOp(Opcode::INT, pack(n.token.intVal()));
        }
        else if (n.type == &FloatType::instance) {
            chunk.addOp(Opcode::FLOAT, pack(n.token.floatVal()));
        }
        else if (n.type == &BoolType::instance) {
            chunk.addOp(Opcode::INT, n.token.boolVal() ? 1 : 0);
        }
        else if (n.type == &ClassDecl::String) {
            index = chunk.addConstant(VMValue(n.token.value.c_str()));
            chunk.addOp(Opcode::Const, index);
        }
        else if (n.type == &NullType::instance) {
            chunk.addOp(Opcode::Null);
        }
        else error(n, "Invalid literal type");
    }

    void ByteCodeCompiler::visit(BlockStmt& n) {
        visitChildren(n.stmts);
    }

    void ByteCodeCompiler::visit(BinopExpr& n) {
        visitChild(n.left);
        visitChild(n.right);
        switch (n.op) {
            case TokenType::Plus:
            chunk.addOp(Opcode::Add);
            break;
            case TokenType::Minus:
            chunk.addOp(Opcode::Sub);
            break;
            case TokenType::Asterisk:
            chunk.addOp(Opcode::Mul);
            break;
            case TokenType::Slash:
            chunk.addOp(Opcode::Div);
            break;
            case TokenType::EqualsEquals:
            chunk.addOp(Opcode::CmpEQ);
            break;
            case TokenType::ExclamationMarkEquals:
            chunk.addOp(Opcode::CmpNE);
            break;
            case TokenType::LessThan:
            chunk.addOp(Opcode::CmpLT);
            break;
            case TokenType::LessThanEquals:
            chunk.addOp(Opcode::CmpLTE);
            break;
            case TokenType::GreaterThan:
            chunk.addOp(Opcode::CmpGT);
            break;
            case TokenType::GreaterThanEquals:
            chunk.addOp(Opcode::CmpGTE);
            break;
            case TokenType::AmpAmp:
            chunk.addOp(Opcode::AndL);
            break;
            case TokenType::PipePipe:
            chunk.addOp(Opcode::OrL);
            break;
            default:
            error(n, "Invalid binary operator");
        }
    }

    void ByteCodeCompiler::visit(ScopeExpr& n) {
        visitChild(n.scopeTarget);
        if (auto fun = n.node->as<FuncDecl>()) {
            auto index = chunk.addOp(Opcode::Const, 255);
            functionFixups[index] = fun;
        }
        else if (auto field = n.node->as<FieldDecl>()) {
            chunk.addOp(Opcode::Field, field->index);
        }
        else if (auto ee = n.node->as<EnumElement>()) {
            chunk.addOp(Opcode::INT, ee->index);
        }
        else {
            error(n, "Invalid scope expr");
        }
    }

    void ByteCodeCompiler::visit(IfStmt& n) {
        visitChild(n.condition);
        auto pos = chunk.addOp(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.trueBranch);
        auto index = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos, index);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
    }

    void ByteCodeCompiler::visit(FieldDecl& n) {
    }

    void ByteCodeCompiler::visit(NewExpr& n) {
        chunk.addOp(Opcode::New, n.type->as<ClassDecl>()->fields.size());
    }

    void ByteCodeCompiler::visit(AssignExpr& n) {
        visitChild(n.right);
        chunk.addOp(Opcode::Repeat);

        if (auto var = n.left->node->as<VarDecl>()) {
            chunk.addOp(Opcode::StoreVar, var->index);
        }
        else if (auto par = n.left->node->as<Param>()) {
            chunk.addOp(Opcode::StoreParam, par->index);
        }
        else if (auto field = n.left->node->as<FieldDecl>()) {
            visitChild(n.left);
            chunk.addOp(Opcode::StoreField, field->index);
        }
    }

    void ByteCodeCompiler::visit(WhileStmt& n) {
        auto startPos = chunk.opcodes.size();
        visitChild(n.condition);
        auto pos = chunk.addOp(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.body);
        chunk.addOp(Opcode::Const, chunk.addConstant(VMValue(int64_t(startPos))));
        chunk.addOp(Opcode::Jmp);
        auto index = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos, index);
    }

    void ByteCodeCompiler::visit(PostfixExpr& n) {
        
    }

    void ByteCodeCompiler::visit(UnaryExpr& n) {
        visitChild(n.target);
        switch (n.op) {
            case TokenType::Minus:
            chunk.addOp(Opcode::INT, pack(-1));
            chunk.addOp(Opcode::Mul);
            return;
            break;

            case TokenType::ExclamationMark:
            chunk.addOp(Opcode::Not);
            return;
            break;
        }
        error(n, "Unhandled unary prefix operator '" + getTokenName(n.op) + "'.");
    }

    void ByteCodeCompiler::visit(EnumDecl& n) {
        visitChildren(n.elements);
    }

    void ByteCodeCompiler::visit(EnumElement& n) {
    }

    void ByteCodeCompiler::error(Node& n, const std::string& msg) {
        throw Exception("ByteCodeCompiler: " + msg);
    }
}