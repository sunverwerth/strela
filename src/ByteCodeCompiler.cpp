#include "ByteCodeCompiler.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "ByteCodeChunk.h"
#include "Opcode.h"
#include "Scope.h"
#include "SourceFile.h"

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

    void ByteCodeCompiler::addFixup(int address, FuncDecl* function) {
        functionFixups[address] = function;
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
        auto oldclass = _class;
        _class = &n;
        visitChildren(n.methods);
        _class = oldclass;
    }

    void ByteCodeCompiler::visit(FuncDecl& n) {
        auto oldfunc = function;
        function = &n;
        n.opcodeStart = chunk.opcodes.size();
        std::stringstream sstr;
        if (_class) {
            sstr << _class->name << ".";
        }
        sstr << n.name << n.type->name;
        chunk.addFunction(n.opcodeStart, sstr.str());

        for (size_t i = 0; i < n.params.size(); ++i) {
            n.params[i]->index = _class ? i + 1 : i;
        }

        visitChildren(n.params);
        if (n.numVariables > 0) {
            chunk.addOp(Opcode::GrowStack, n.numVariables);
        }
        visitChildren(n.stmts);

        if (n.isExternal) {
            for (int i = n.params.size() - 1; i >= 0; --i) {
                chunk.addOp(Opcode::Param, i);
            }
            auto index = chunk.addForeignFunction(n.name);
            chunk.addOp(Opcode::INT, index);
            chunk.addOp(Opcode::NativeCall);
            if (n.type->returnType != &VoidType::instance) {
                chunk.addOp(Opcode::Return, n.params.size() + (_class ? 1 : 0));
            }
            else {
                chunk.addOp(Opcode::ReturnVoid, n.params.size() + (_class ? 1 : 0));
            }
        }
        else if (!n.returns) {
            chunk.addOp(Opcode::ReturnVoid, n.params.size() + (_class ? 1 : 0));
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
            addFixup(index, fun);
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

        if (!n.callTarget->context && n.callTarget->node && n.callTarget->node->as<FuncDecl>() && n.callTarget->node->as<FuncDecl>()->name == "print") {
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
            chunk.addOp(Opcode::Return, function->params.size() + (_class ? 1 : 0));
        }
        else {
            chunk.addOp(Opcode::ReturnVoid, function->params.size() + (_class ? 1 : 0));
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

    void ByteCodeCompiler::visit(IsExpr& n) {
        visitChild(n.target);
        chunk.addOp(Opcode::Field, 0);
        chunk.addOp(Opcode::INT, n.typeTag);
        chunk.addOp(Opcode::CmpEQ);
    }

    void ByteCodeCompiler::visit(CastExpr& n) {
        auto totype = n.targetType;
        auto fromtype = n.sourceExpr->type;

        auto toiface = totype->as<InterfaceDecl>();
        auto fromclass = fromtype->as<ClassDecl>();

        auto tounion = totype->as<UnionType>();
        auto fromunion = fromtype->as<UnionType>();

        if (fromclass && toiface && n.implementation) {
            chunk.addOp(Opcode::New, toiface->methods.size() + 1);
            chunk.addOp(Opcode::Repeat);
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::Swap);
            chunk.addOp(Opcode::StoreField, 0);
            for(size_t i = 0; i < toiface->methods.size(); ++i) {
                chunk.addOp(Opcode::Repeat);
                auto index = chunk.addOp(Opcode::Const, 255);
                addFixup(index, n.implementation->classMethods[i]);
                chunk.addOp(Opcode::Swap);
                chunk.addOp(Opcode::StoreField, i + 1);
            }
        }
        else if (tounion) {
            auto tag = tounion->getTypeTag(fromtype);
            chunk.addOp(Opcode::New, 2);
            
            chunk.addOp(Opcode::Repeat);
            chunk.addOp(Opcode::INT, tag);
            chunk.addOp(Opcode::Swap);
            chunk.addOp(Opcode::StoreField, 0);

            chunk.addOp(Opcode::Repeat);
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::Swap);
            chunk.addOp(Opcode::StoreField, 1);
        }
        else if (fromunion) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::Field, 1);
        }
        else if (totype->as<IntType>() && fromtype->as<FloatType>()) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::F2I);
        }
        else if (fromtype->as<IntType>() && totype->as<FloatType>()) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::I2F);
        }
        else {
            visitChild(n.sourceExpr);
        }
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
            addFixup(index, fun);
        }
        else if (auto im = n.node->as<InterfaceMethodDecl>()) {
            chunk.addOp(Opcode::Repeat);
            chunk.addOp(Opcode::Field, 1 + im->index);
            chunk.addOp(Opcode::Swap);
            chunk.addOp(Opcode::Field, 0);
            chunk.addOp(Opcode::Swap);
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

    void ByteCodeCompiler::visit(ArrayLitExpr& n) {
        chunk.addOp(Opcode::New, 1 + n.elements.size());
        chunk.addOp(Opcode::Repeat);
        chunk.addOp(Opcode::INT, n.elements.size());
        chunk.addOp(Opcode::Swap);
        chunk.addOp(Opcode::StoreField, 0);
        size_t i = 0;
        for (auto&& el: n.elements) {
            chunk.addOp(Opcode::Repeat);
            visitChild(el);
            chunk.addOp(Opcode::Swap);
            chunk.addOp(Opcode::StoreField, ++i);
        }

    }

    void ByteCodeCompiler::visit(SubscriptExpr& n) {
        for (int i = n.arguments.size() - 1; i >= 0; --i) {
            visitChild(n.arguments[i]);
        }
        visitChild(n.callTarget);

        if (n.subscriptFunction) {
            auto index = chunk.addOp(Opcode::Const, 255);
            addFixup(index, n.subscriptFunction);
            chunk.addOp(Opcode::Call);
        }
        else {
            chunk.addOp(Opcode::FieldInd, 1);
        }
    }

    void ByteCodeCompiler::visit(IfStmt& n) {
        visitChild(n.condition);
        auto pos = chunk.addOp(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.trueBranch);
        auto pos2 = chunk.addOp(Opcode::Const, 0);
        chunk.addOp(Opcode::Jmp);

        auto index = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos, index);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
        auto index2 = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos2, index2);
    }

    void ByteCodeCompiler::visit(FieldDecl& n) {
    }

    void ByteCodeCompiler::visit(NewExpr& n) {
        chunk.addOp(Opcode::New, n.type->as<ClassDecl>()->fields.size());
        if (n.initMethod) {
            chunk.addOp(Opcode::Repeat);
            for (int i = n.arguments.size() - 1; i >= 0; --i) {
                n.arguments[i]->accept(*this);
                chunk.addOp(Opcode::Swap);
            }
            auto index = chunk.addOp(Opcode::Const, 255);
            addFixup(index, n.initMethod);
            chunk.addOp(Opcode::Call);
        }
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
            visitChild(n.left->context);
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

    void ByteCodeCompiler::visit(ThisExpr& n) {
        chunk.addOp(Opcode::Param, 0);
    }

    void ByteCodeCompiler::error(Node& n, const std::string& msg) {
        if (n.source) {
            throw Exception(n.source->filename + ":" + std::to_string(n.line) + ":" + std::to_string(n.column) + " Error: " + msg);
        }
        else {
            throw Exception("Error: " + msg);
        }
    }
}