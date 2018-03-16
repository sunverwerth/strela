#include "ByteCodeCompiler.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "ByteCodeChunk.h"
#include "Opcode.h"
#include "Scope.h"
#include "Types/types.h"

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
            auto index = chunk.addConstant(VMValue((uint64_t)fixup.second->opcodeStart));
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
        sstr << n.name << "(";
        for (auto&& param: n.params) {
            sstr << param->declType->name;
            if (&param != &n.params.back()) {
                sstr << ", ";
            }
        }
        sstr << "): " << n.returnType->name;
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
        if (tryCompileAsConst(n)) return;

        if (n.referencedFunction) {
            auto index = chunk.addOp(Opcode::Const, 255);
            functionFixups[index] = n.referencedFunction;
        }
        else if (n.referencedParam) {
            chunk.addOp(Opcode::Param, n.referencedParam->index);
        }
        else if (n.referencedVar) {
            chunk.addOp(Opcode::Var, n.referencedVar->index);
        }
    }

    void ByteCodeCompiler::visit(ExprStmt& n) {
        visitChild(n.expression);
        if (n.expression->type != Types::_void) {
            chunk.addOp(Opcode::Pop);
        }
    }

    void ByteCodeCompiler::visit(CallExpr& n) {
        for (size_t i = n.arguments.size(); i > 0; --i) {
            n.arguments[i - 1]->accept(*this);
        }

        auto fun = n.callTarget->referencedFunction;
        if (fun->name == "print") {
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
        if (n.type == Types::u8) {
            chunk.addOp(Opcode::U8, pack(n.staticValue.u8));
        }
        else if (n.type == Types::u16) {
            chunk.addOp(Opcode::U16, pack(n.staticValue.u16));
        }
        else if (n.type == Types::u32) {
            chunk.addOp(Opcode::U32, pack(n.staticValue.u32));
        }
        else if (n.type == Types::u64) {
            index = chunk.addConstant(VMValue(n.staticValue.u64));
            chunk.addOp(Opcode::Const, index);
        }
        else if (n.type == Types::i8) {
            chunk.addOp(Opcode::I8, pack(n.staticValue.i8));
        }
        else if (n.type == Types::i16) {
            chunk.addOp(Opcode::I16, pack(n.staticValue.i16));
        }
        else if (n.type == Types::i32) {
            chunk.addOp(Opcode::I32, pack(n.staticValue.i32));
        }
        else if (n.type == Types::i64) {
            index = chunk.addConstant(VMValue(n.staticValue.i64));
            chunk.addOp(Opcode::Const, index);
        }
        else if (n.type == Types::f32) {
            chunk.addOp(Opcode::F32, pack(n.staticValue.f32));
        }
        else if (n.type == Types::f64) {
            index = chunk.addConstant(VMValue(n.staticValue.f64));
            chunk.addOp(Opcode::Const, index);
        }
        else if (n.type == Types::boolean) {
            chunk.addOp(Opcode::U8, n.staticValue.boolean ? 1 : 0);
        }
        else if (n.type == Types::string) {
            index = chunk.addConstant(VMValue(n.staticValue.string));
            chunk.addOp(Opcode::Const, index);
        }
        else if (n.type == Types::null) {
            chunk.addOp(Opcode::Null);
        }
        else error(n, "Invalid literal type");
    }

    void ByteCodeCompiler::visit(BlockStmt& n) {
        visitChildren(n.stmts);
    }

    void ByteCodeCompiler::visit(BinopExpr& n) {
        if (tryCompileAsConst(n)) return;

        visitChild(n.left);
        visitChild(n.right);
        switch (n.startToken.type) {
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
        if (n.referencedFunction) {
            auto index = chunk.addOp(Opcode::Const, 255);
            functionFixups[index] = n.referencedFunction;
        }
        else if (n.referencedField) {
            chunk.addOp(Opcode::Field, n.referencedField->index);
        }
        else if (n.referencedEnumElement) {
            chunk.addOp(Opcode::I32, n.referencedEnumElement->index);
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
        auto index = chunk.addConstant(VMValue((uint64_t)chunk.opcodes.size()));
        chunk.writeArgument(pos, index);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
    }

    void ByteCodeCompiler::visit(FieldDecl& n) {
    }

    void ByteCodeCompiler::visit(NewExpr& n) {
        chunk.addOp(Opcode::New, n.type->as<ClassType>()->_class->fields.size());
    }

    void ByteCodeCompiler::visit(AssignExpr& n) {
        visitChild(n.right);
        chunk.addOp(Opcode::Repeat);
        if (n.left->referencedVar) {
            chunk.addOp(Opcode::StoreVar, n.left->referencedVar->index);
        }
        else if (n.left->referencedParam) {
            chunk.addOp(Opcode::StoreParam, n.left->referencedParam->index);
        }
        else if (n.left->referencedField) {
            if (n.left->nodeParent) {
                visitChild(n.left->nodeParent);
                chunk.addOp(Opcode::StoreField, n.left->referencedField->index);
            }
            else {
                error(n, "Invalid target");
            }
        }
        else {
            error(n, "Invalid target");
        }
    }

    void ByteCodeCompiler::visit(IdTypeExpr&) {
    }

    void ByteCodeCompiler::visit(WhileStmt& n) {
        auto startPos = chunk.opcodes.size();
        visitChild(n.condition);
        auto pos = chunk.addOp(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.body);
        chunk.addOp(Opcode::Const, chunk.addConstant(VMValue((uint64_t)startPos)));
        chunk.addOp(Opcode::Jmp);
        auto index = chunk.addConstant(VMValue((uint64_t)chunk.opcodes.size()));
        chunk.writeArgument(pos, index);
    }

    void ByteCodeCompiler::visit(PostfixExpr& n) {
        visitChild(n.target);
        chunk.addOp(Opcode::Repeat);
        chunk.addOp(Opcode::U8, 1);
        if (n.startToken.type == TokenType::PlusPlus) {
            chunk.addOp(Opcode::Add);
        }
        else if (n.startToken.type == TokenType::MinusMinus) {
            chunk.addOp(Opcode::Sub);
        }

        if (n.referencedVar) {
            chunk.addOp(Opcode::StoreVar, n.referencedVar->index);
        }
        else if (n.referencedParam) {
            chunk.addOp(Opcode::StoreParam, n.referencedParam->index);
        }
        else if (n.referencedField) {
            if (n.target->nodeParent) {
                visitChild(n.target->nodeParent);
                chunk.addOp(Opcode::StoreField, n.referencedField->index);
            }
            else {
                error(n, "Invalid target");
            }
        }
        else {
            error(n, "Invalid target");
        }
    }

    void ByteCodeCompiler::visit(UnaryExpr& n) {
        if (tryCompileAsConst(n)) return;

        visitChild(n.target);
        switch (n.startToken.type) {
            case TokenType::Minus:
            chunk.addOp(Opcode::I32, pack(-1));
            chunk.addOp(Opcode::Mul);
            return;
            break;

            case TokenType::ExclamationMark:
            chunk.addOp(Opcode::Not);
            return;
            break;
        }
        error(n, "Unhandled unary prefix operator '" + n.startToken.value + "'.");
    }

    void ByteCodeCompiler::visit(EnumDecl& n) {
        visitChildren(n.elements);
    }

    void ByteCodeCompiler::visit(EnumElement& n) {
    }

    bool ByteCodeCompiler::tryCompileAsConst(Expr& n) {
        if (n.isStatic) {
            if (n.type == Types::i32) {
                chunk.addOp(Opcode::I32, pack(n.staticValue.i32));
            }
            else {
                return false;
            }
            return true;
        }
        return false;
    }

    void ByteCodeCompiler::error(Node& n, const std::string& msg) {
        throw Exception("ByteCodeCompiler: " + msg);
    }
}