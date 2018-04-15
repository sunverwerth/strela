// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "ByteCodeCompiler.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "VM/ByteCodeChunk.h"
#include "VM/Opcode.h"
#include "Scope.h"
#include "SourceFile.h"

#include <sstream>
#include <cstring>

namespace Strela {

    size_t align(size_t offset, size_t alignment) {
        return offset + (offset % alignment);
    }

    VMType* ByteCodeCompiler::mapType(TypeDecl* type) {
        auto it = typeMap.find(type);
        if (it != typeMap.end()) return it->second;
        
        auto vmtype = new VMType;
        vmtype->index = chunk.types.size();
        chunk.types.push_back(vmtype);
        typeMap.insert(std::make_pair(type, vmtype));

        vmtype->name = type->name;
        
        vmtype->isObject = (type->as<ClassDecl>() || type->as<InterfaceDecl>() || type->as<UnionType>()) && type != &ClassDecl::String;
        vmtype->isArray = type->as<ArrayType>();

        if (vmtype->isArray) {
            vmtype->objectAlignment = sizeof(void*);
            vmtype->size = sizeof(void*);
            vmtype->alignment = sizeof(void*);
            vmtype->arrayType = mapType(type->as<ArrayType>()->baseType);
            vmtype->fields.push_back({
                mapType(&IntType::u64),
                0
            });
        }
        else if (vmtype->isObject) {
            if (auto cls = type->as<ClassDecl>()) {
                vmtype->size = sizeof(void*);
                vmtype->alignment = sizeof(void*);
                size_t offset = 0;
                size_t alignment = 1;
                for (auto&& field: cls->fields) {
                    auto ftype = mapType(field->type);
                    if (ftype->alignment > alignment) alignment = ftype->alignment;
                    offset = align(offset, alignment);
                    vmtype->fields.push_back({
                        ftype,
                        offset
                    });
                    if (ftype->isArray || ftype->isObject) {
                        offset += sizeof(void*);
                    }
                    else {
                        offset += ftype->size;
                    }
                }
                vmtype->objectSize = align(offset, alignment);
                vmtype->objectAlignment = alignment;
            }
            else if (auto iface = type->as<InterfaceDecl>()) {
                vmtype->size = sizeof(void*);
                vmtype->alignment = sizeof(void*);
                vmtype->objectSize = sizeof(void*) + iface->methods.size() * sizeof(void*);
                vmtype->objectAlignment = sizeof(void*);
            }
            else if (auto un = type->as<UnionType>()) {
                vmtype->size = sizeof(void*);
                vmtype->alignment = sizeof(void*);
                vmtype->objectSize = 16;
                vmtype->objectAlignment = 8;
                vmtype->fields.push_back({mapType(&IntType::u64), 0});
                vmtype->fields.push_back({mapType(&IntType::u64), 8});
            }
        }
        else {
            if (auto intt = type->as<IntType>()) {
                vmtype->size = intt->bytes;
                vmtype->alignment = intt->bytes;
            }
            else if (type->as<EnumDecl>()) {
                vmtype->size = 4;
                vmtype->alignment = 4;
            }
            else if (type == &FloatType::f32) {
                vmtype->size = 4;
                vmtype->alignment = 4;
            }
            else if (type == &FloatType::f64) {
                vmtype->size = 8;
                vmtype->alignment = 8;
            }
            else if (type == &ClassDecl::String) {
                vmtype->size = sizeof(void*);
                vmtype->alignment = sizeof(void*);
            }
            else if (type == &PointerType::instance) {
                vmtype->size = sizeof(void*);
                vmtype->alignment = sizeof(void*);
            }
        }
        return vmtype;
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
                _class = fixup.second->_class;
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
        if (!n.genericParams.empty() && n.genericArguments.empty()) {
            visitChildren(n.reifiedClasses);
        }
        else {
            visitChildren(n.methods);
        }
        _class = oldclass;
    }

    void ByteCodeCompiler::visit(FuncDecl& n) {
        auto oldfunc = function;
        function = &n;
        n.opcodeStart = chunk.opcodes.size();
        std::stringstream sstr;
        if (n._class) {
            sstr << n._class->name << ".";
        }
        sstr << n.name << n.type->name;
        chunk.addFunction(n.opcodeStart, sstr.str());

        for (size_t i = 0; i < n.params.size(); ++i) {
            n.params[i]->index = n._class ? i + 1 : i;
        }

        if (n.numVariables > 0 ) {
            chunk.addOp<uint8_t>(Opcode::Grow, n.numVariables);
        }

        visitChildren(n.params);
        visitChildren(n.stmts);

        if (n.isExternal) {
            /*for (int i = n.params.size() - 1; i >= 0; --i) {
                chunk.addOp<uint8_t>(Opcode::Var, i);
            }
            auto index = chunk.addForeignFunction(n);
            chunk.addOp<uint64_t>(Opcode::I64, index);
            chunk.addOp(Opcode::NativeCall);
            if (n.type->returnType != &VoidType::instance) {
                chunk.addOp(Opcode::Return);
            }
            else {
                chunk.addOp(Opcode::ReturnVoid);
            }*/
        }
        else if (!n.returns) {
            chunk.addOp(Opcode::ReturnVoid);
        }

        function = oldfunc;
    }

    void ByteCodeCompiler::visit(VarDecl& n) {
        mapType(n.type);
        if (n.initializer) {
            n.initializer->accept(*this);
            chunk.addOp<uint8_t>(Opcode::StoreVar, function->params.size() + (_class ? 1 : 0) + n.index);
        }
    }

    void ByteCodeCompiler::visit(FieldDecl& n) {
        mapType(n.type);
    }

    void ByteCodeCompiler::visit(Param& n) {
        mapType(n.type);
    }

    void ByteCodeCompiler::visit(IdExpr& n) {
        if (auto fun = n.node->as<FuncDecl>()) {
            if (fun->isExternal) {
                auto index = chunk.addForeignFunction(*fun);
                chunk.addOp<uint64_t>(Opcode::I64, index);
            }
            else {
                auto index = chunk.addOp<uint8_t>(Opcode::Const, 255);
                addFixup(index, fun);
            }
        }
        else if (auto param = n.node->as<Param>()) {
            chunk.addOp<uint8_t>(Opcode::Var, param->index);
        }
        else if (auto var = n.node->as<VarDecl>()) {
            chunk.addOp<uint8_t>(Opcode::Var, function->params.size() + (_class ? 1 : 0) + var->index);
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
            auto t = n.arguments.front()->type;
            if (t->as<IntType>()) {
                chunk.addOp(Opcode::PrintI);
            }
            else if (t == &FloatType::f32) {
                chunk.addOp(Opcode::PrintF32);
            }
            else if (t == &FloatType::f64) {
                chunk.addOp(Opcode::PrintF64);
            }
            else if (t == &NullType::instance) {
                chunk.addOp(Opcode::PrintN);
            }
            else if (t == &ClassDecl::String) {
                chunk.addOp(Opcode::PrintS);
            }
            else if (t->as<ClassDecl>()) {
                chunk.addOp(Opcode::PrintO);
            }
            else if (t->as<BoolType>()) {
                chunk.addOp(Opcode::PrintB);
            }
            else {
                chunk.addOp(Opcode::PrintN);
            }
        }
        else {
            visitChild(n.callTarget);
            if (n.callTarget->node && n.callTarget->node->as<FuncDecl>() && n.callTarget->node->as<FuncDecl>()->isExternal) {
                chunk.addOp(Opcode::NativeCall);
            }
            else {
                chunk.addOp<uint8_t>(Opcode::Call, n.callTarget->type->as<FuncType>()->paramTypes.size() + (n.callTarget->context ? 1 : 0));
            }
        }
    }

    void ByteCodeCompiler::visit(RetStmt& n) {
        if (n.expression) {
            visitChild(n.expression);
            chunk.addOp(Opcode::Return);
        }
        else {
            chunk.addOp(Opcode::ReturnVoid);
        }
    }

    void ByteCodeCompiler::visit(LitExpr& n) {
        int index = 0;
        if (n.type == &IntType::u8) {
            chunk.addOp<uint8_t>(Opcode::U8, n.token.intVal());
        }
        else if (n.type == &IntType::u16) {
            chunk.addOp<uint16_t>(Opcode::U16, n.token.intVal());
        }
        else if (n.type == &IntType::u32) {
            chunk.addOp<uint32_t>(Opcode::U32, n.token.intVal());
        }
        else if (n.type == &IntType::u64) {
            chunk.addOp<uint64_t>(Opcode::U64, n.token.intVal());
        }
        else if (n.type == &IntType::i8) {
            chunk.addOp<int8_t>(Opcode::I8, n.token.intVal());
        }
        else if (n.type == &IntType::i16) {
            chunk.addOp<int16_t>(Opcode::I16, n.token.intVal());
        }
        else if (n.type == &IntType::i32) {
            chunk.addOp<int32_t>(Opcode::I32, n.token.intVal());
        }
        else if (n.type == &IntType::i64) {
            chunk.addOp<int64_t>(Opcode::I64, n.token.intVal());
        }
        else if (n.type == &FloatType::f32) {
            chunk.addOp<float>(Opcode::F32, n.token.floatVal());
        }
        else if (n.type == &FloatType::f64) {
            chunk.addOp<double>(Opcode::F64, n.token.floatVal());
        }
        else if (n.type == &BoolType::instance) {
            chunk.addOp<uint8_t>(Opcode::U8, (n.token.boolVal() ? 1 : 0));
        }
        else if (n.type == &ClassDecl::String) {
            index = chunk.addConstant(VMValue(n.token.value.c_str()));
            chunk.addOp<uint8_t>(Opcode::Const, index);
        }
        else if (n.type == &NullType::instance) {
            chunk.addOp(Opcode::Null);
        }
        else error(n, "Invalid literal type");
    }

    void ByteCodeCompiler::visit(IsExpr& n) {
        visitChild(n.target);
        chunk.addOp<uint8_t>(Opcode::Field64, 0);
        chunk.addOp<int64_t>(Opcode::I64, n.typeTag);
        chunk.addOp(Opcode::CmpEQ);
    }

    void ByteCodeCompiler::visit(CastExpr& n) {
        auto totype = n.targetType;
        auto fromtype = n.sourceExpr->type;

        if (fromtype == totype) {
            visitChild(n.sourceExpr);
            return;
        }

        auto toiface = totype->as<InterfaceDecl>();
        auto fromclass = fromtype->as<ClassDecl>();

        auto tounion = totype->as<UnionType>();
        auto fromunion = fromtype->as<UnionType>();

        if (fromclass && toiface && n.implementation) {
            chunk.addOp<int64_t>(Opcode::I64, mapType(toiface)->index);
            chunk.addOp(Opcode::New);
            chunk.addOp(Opcode::Repeat);
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(Opcode::StoreField64, 0);
            for(size_t i = 0; i < toiface->methods.size(); ++i) {
                chunk.addOp(Opcode::Repeat);
                auto index = chunk.addOp<uint8_t>(Opcode::Const, 255);
                addFixup(index, n.implementation->classMethods[i]);
                chunk.addOp(Opcode::Swap);
                chunk.addOp<uint8_t>(Opcode::StoreField64, i * 8 + 8);
            }
        }
        else if (tounion) {
            auto tag = tounion->getTypeTag(fromtype);
            chunk.addOp<int64_t>(Opcode::I64, mapType(tounion)->index);
            chunk.addOp(Opcode::New);
            
            chunk.addOp(Opcode::Repeat);
            chunk.addOp<int64_t>(Opcode::U64, tag);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(Opcode::StoreField64, 0);

            chunk.addOp(Opcode::Repeat);
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(Opcode::StoreField64, 8);
        }
        else if (fromunion) {
            visitChild(n.sourceExpr);
            chunk.addOp<uint8_t>(Opcode::Field64, 8);
        }
        else if (fromtype == &FloatType::f32 && totype->as<IntType>()) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::F32tI64);
        }
        else if (fromtype == &FloatType::f64 && totype->as<IntType>()) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::F64tI64);
        }
        else if (fromtype->as<IntType>() && totype == &FloatType::f32) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::I64tF32);
        }
        else if (fromtype->as<IntType>() && totype == &FloatType::f64) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::I64tF64);
        }
        else if (fromtype == &FloatType::f32 && totype == &FloatType::f64) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::F32tF64);
        }
        else if (fromtype == &FloatType::f64 && totype == &FloatType::f32) {
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::F64tF32);
        }
        else {
            visitChild(n.sourceExpr);
        }
    }

    void ByteCodeCompiler::visit(BlockStmt& n) {
        visitChildren(n.stmts);
    }

    void ByteCodeCompiler::visit(BinopExpr& n) {
        if (n.function) {
            visitChild(n.right);
            visitChild(n.left);
            auto index = chunk.addOp<uint8_t>(Opcode::Const, 255);
            addFixup(index, n.function);
            chunk.addOp<uint8_t>(Opcode::Call, 2);
            return;
        }

        if (n.op == TokenType::AmpAmp) {
            visitChild(n.left);
            auto const1 = chunk.addOp<uint8_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::JmpIfNot);
            chunk.addOp<uint8_t>(Opcode::U8, 1);
            visitChild(n.right);
            chunk.addOp(Opcode::AndL);
            auto const2 = chunk.addOp<uint8_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::Jmp);
            chunk.writeArgument(const1, chunk.addConstant(VMValue((int64_t)chunk.opcodes.size())));
            chunk.addOp<uint8_t>(Opcode::U8, 0);
            chunk.writeArgument(const2, chunk.addConstant(VMValue((int64_t)chunk.opcodes.size())));            
        }
        else if (n.op == TokenType::PipePipe) {
            visitChild(n.left);
            auto const1 = chunk.addOp<uint8_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::JmpIf);
            chunk.addOp<uint8_t>(Opcode::U8, 0);
            visitChild(n.right);
            chunk.addOp(Opcode::OrL);
            auto const2 = chunk.addOp<uint8_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::Jmp);
            chunk.writeArgument(const1, chunk.addConstant(VMValue((int64_t)chunk.opcodes.size())));
            chunk.addOp<uint8_t>(Opcode::U8, 1);
            chunk.writeArgument(const2, chunk.addConstant(VMValue((int64_t)chunk.opcodes.size())));
        }
        else {
            visitChild(n.left);
            visitChild(n.right);
            switch (n.op) {
                case TokenType::Plus:
                if (n.left->type == &ClassDecl::String) {
                    if (n.right->type == &ClassDecl::String) {
                        chunk.addOp(Opcode::ConcatSS);
                    }
                    else {
                        chunk.addOp(Opcode::ConcatSI);
                    }
                }
                else if (n.left->type->as<IntType>()) {
                    chunk.addOp(Opcode::AddI);
                }
                else if (n.left->type == &FloatType::f32) {
                    chunk.addOp(Opcode::AddF32);
                }
                else if (n.left->type == &FloatType::f64) {
                    chunk.addOp(Opcode::AddF64);
                }
                else {
                    error(n, "Invalid op");
                }
                break;
                case TokenType::Minus:
                if (n.left->type->as<IntType>()) {
                    chunk.addOp(Opcode::SubI);
                }
                else if (n.left->type == &FloatType::f32) {
                    chunk.addOp(Opcode::SubF32);
                }
                else if (n.left->type == &FloatType::f64) {
                    chunk.addOp(Opcode::SubF64);
                }
                else {
                    error(n, "Invalid op");
                }
                break;
                case TokenType::Asterisk:
                if (n.left->type->as<IntType>()) {
                    chunk.addOp(Opcode::MulI);
                }
                else if (n.left->type == &FloatType::f32) {
                    chunk.addOp(Opcode::MulF32);
                }
                else if (n.left->type == &FloatType::f64) {
                    chunk.addOp(Opcode::MulF64);
                }
                else {
                    error(n, "Invalid op");
                }
                break;
                case TokenType::Slash:
                if (n.left->type->as<IntType>()) {
                    chunk.addOp(Opcode::DivI);
                }
                else if (n.left->type == &FloatType::f32) {
                    chunk.addOp(Opcode::DivF32);
                }
                else if (n.left->type == &FloatType::f64) {
                    chunk.addOp(Opcode::DivF64);
                }
                else {
                    error(n, "Invalid op");
                }
                break;
                case TokenType::EqualsEquals:
                if (n.left->type == &ClassDecl::String && n.right->type == &ClassDecl::String) {
                    chunk.addOp(Opcode::CmpEQS);
                }
                else {
                    chunk.addOp(Opcode::CmpEQ);
                }
                break;
                case TokenType::ExclamationMarkEquals:
                chunk.addOp(Opcode::CmpNE);
                break;
                case TokenType::LessThan:
                if (n.left->type->as<IntType>()) {
                    chunk.addOp(Opcode::CmpLTI);
                }
                else if (n.left->type == &FloatType::f32) {
                    chunk.addOp(Opcode::CmpLTF32);
                }
                else if (n.left->type == &FloatType::f64) {
                    chunk.addOp(Opcode::CmpLTF64);
                }
                else {
                    error(n, "Invalid op");
                }
                break;
                case TokenType::LessThanEquals:
                chunk.addOp(Opcode::CmpLTE);
                break;
                case TokenType::GreaterThan:
                if (n.left->type->as<IntType>()) {
                    chunk.addOp(Opcode::CmpGTI);
                }
                else if (n.left->type == &FloatType::f32) {
                    chunk.addOp(Opcode::CmpGTF32);
                }
                else if (n.left->type == &FloatType::f64) {
                    chunk.addOp(Opcode::CmpGTF64);
                }
                else {
                    error(n, "Invalid op");
                }
                break;
                case TokenType::GreaterThanEquals:
                chunk.addOp(Opcode::CmpGTE);
                break;
                default:
                error(n, "Invalid binary operator");
            }
        }
    }

    void ByteCodeCompiler::visit(ScopeExpr& n) {
        visitChild(n.scopeTarget);
        if (auto fun = n.node->as<FuncDecl>()) {
            auto index = chunk.addOp<uint8_t>(Opcode::Const, 255);
            addFixup(index, fun);
        }
        else if (auto im = n.node->as<InterfaceMethodDecl>()) {
            chunk.addOp(Opcode::Repeat);
            chunk.addOp<uint8_t>(Opcode::Field64, 8 + im->index * 8);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(Opcode::Field64, 0);
            chunk.addOp(Opcode::Swap);
        }
        else if (auto field = n.node->as<FieldDecl>()) {
            auto t = mapType(n.scopeTarget->type);
            auto ft = mapType(field->type);
            Opcode op;
            switch (ft->size) {
                case 1: op = Opcode::Field8; break;
                case 2: op = Opcode::Field16; break;
                case 4: op = Opcode::Field32; break;
                case 8: op = Opcode::Field64; break;
            }
            chunk.addOp<uint8_t>(op, t->fields[field->index].offset);
        }
        else if (auto ee = n.node->as<EnumElement>()) {
            chunk.addOp<int64_t>(Opcode::I64, ee->index);
        }
        else {
            error(n, "Invalid scope expr");
        }
    }

    void ByteCodeCompiler::visit(ArrayLitExpr& n) {
        chunk.addOp<uint64_t>(Opcode::U64, mapType(n.type)->index);
        chunk.addOp<uint64_t>(Opcode::U64, n.elements.size());
        chunk.addOp(Opcode::Array);
        chunk.addOp(Opcode::Repeat);
        auto t = mapType(n.type->as<ArrayType>()->baseType);
        Opcode op;
        switch (t->size) {
            case 1: op = Opcode::StoreField8; break;
            case 2: op = Opcode::StoreField16; break;
            case 4: op = Opcode::StoreField32; break;
            case 8: op = Opcode::StoreField64; break;
        }
        size_t i = 8;
        for (auto&& el: n.elements) {
            chunk.addOp(Opcode::Repeat);
            visitChild(el);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(op, i);
            i += t->size;
        }
    }

    void ByteCodeCompiler::visit(SubscriptExpr& n) {
        if (n.subscriptFunction) {
            for (int i = n.arguments.size() - 1; i >= 0; --i) {
                visitChild(n.arguments[i]);
            }
            visitChild(n.callTarget);
            auto index = chunk.addOp<uint8_t>(Opcode::Const, 255);
            addFixup(index, n.subscriptFunction);
            chunk.addOp<uint8_t>(Opcode::Call, n.subscriptFunction->params.size() + 1);
        }
        else {
            auto fieldSize = mapType(n.callTarget->type)->arrayType->size;
            for (int i = n.arguments.size() - 1; i >= 0; --i) {
                visitChild(n.arguments[i]);
                if (fieldSize != 1) {
                    chunk.addOp<uint8_t>(Opcode::U8, fieldSize);
                    chunk.addOp(Opcode::MulI);
                }
            }
            Opcode op;
            switch (fieldSize) {
                case 1: op = Opcode::FieldInd8; break;
                case 2: op = Opcode::FieldInd16; break;
                case 4: op = Opcode::FieldInd32; break;
                case 8: op = Opcode::FieldInd64; break;
            }

            visitChild(n.callTarget);
            chunk.addOp<uint8_t>(op, 8);
        }
    }

    void ByteCodeCompiler::visit(IfStmt& n) {
        visitChild(n.condition);
        auto pos = chunk.addOp<uint8_t>(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.trueBranch);
        auto pos2 = chunk.addOp<uint8_t>(Opcode::Const, 0);
        chunk.addOp(Opcode::Jmp);

        auto index = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos, index);
        if (n.falseBranch) {
            visitChild(n.falseBranch);
        }
        auto index2 = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos2, index2);
    }

    void ByteCodeCompiler::visit(NewExpr& n) {
        if (auto clstype = n.type->as<ClassDecl>()) {
            chunk.addOp<uint64_t>(Opcode::U64, mapType(clstype)->index);
            chunk.addOp(Opcode::New);
            if (n.initMethod) {
                chunk.addOp(Opcode::Repeat);
                for (int i = n.arguments.size() - 1; i >= 0; --i) {
                    n.arguments[i]->accept(*this);
                    chunk.addOp(Opcode::Swap);
                }
                auto index = chunk.addOp<uint8_t>(Opcode::Const, 255);
                addFixup(index, n.initMethod);
                chunk.addOp<uint8_t>(Opcode::Call, n.initMethod->params.size() + 1);
            }
        }
        else if (auto arrtype = n.type->as<ArrayType>()) {
            chunk.addOp<uint64_t>(Opcode::U64, mapType(arrtype)->index);
            visitChild(n.arguments.front());
            chunk.addOp(Opcode::Array);
        }
    }

    void ByteCodeCompiler::visit(AssignExpr& n) {
        visitChild(n.right);
        chunk.addOp(Opcode::Repeat);

        if (n.left->arrayIndex) {
            auto fieldSize = mapType(n.left->context->type)->arrayType->size;
            visitChild(n.left->arrayIndex);
            if (fieldSize != 1) {
                chunk.addOp<uint8_t>(Opcode::U8, fieldSize);
                chunk.addOp(Opcode::MulI);
            }
            Opcode op;
            switch (fieldSize) {
                case 1: op = Opcode::StoreFieldInd8; break;
                case 2: op = Opcode::StoreFieldInd16; break;
                case 4: op = Opcode::StoreFieldInd32; break;
                case 8: op = Opcode::StoreFieldInd64; break;
            }
            visitChild(n.left->context);
            chunk.addOp<uint8_t>(op, 8);
        }
        else if (auto var = n.left->node->as<VarDecl>()) {
                chunk.addOp<uint8_t>(Opcode::StoreVar, function->params.size() + (_class ? 1 : 0) + var->index);
        }
        else if (auto par = n.left->node->as<Param>()) {
            chunk.addOp<uint8_t>(Opcode::StoreVar, par->index);
        }
        else if (auto field = n.left->node->as<FieldDecl>()) {
            visitChild(n.left->context);
            auto t = mapType(n.left->context->type);
            auto ft = mapType(field->type);
            Opcode op;
            switch (ft->size) {
                case 1: op = Opcode::StoreField8; break;
                case 2: op = Opcode::StoreField16; break;
                case 4: op = Opcode::StoreField32; break;
                case 8: op = Opcode::StoreField64; break;
            }
            chunk.addOp<uint8_t>(op, t->fields[field->index].offset);
        }
    }

    void ByteCodeCompiler::visit(WhileStmt& n) {
        auto startPos = chunk.opcodes.size();
        visitChild(n.condition);
        auto pos = chunk.addOp<uint8_t>(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.body);
        chunk.addOp<uint8_t>(Opcode::Const, chunk.addConstant(VMValue(int64_t(startPos))));
        chunk.addOp(Opcode::Jmp);
        auto index = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos, index);
    }

    void ByteCodeCompiler::visit(PostfixExpr& n) {
        visitChild(n.target);
        chunk.addOp(Opcode::Repeat);

        if (n.target->type == &FloatType::f32) {
            chunk.addOp<float>(Opcode::F32, 1);

            if (n.op == TokenType::PlusPlus) {
                chunk.addOp(Opcode::AddF32);
            }
            else if (n.op == TokenType::MinusMinus) {
                chunk.addOp(Opcode::SubF32);
            }
        }
        else if (n.target->type == &FloatType::f64) {
            chunk.addOp<double>(Opcode::F64, 1);

            if (n.op == TokenType::PlusPlus) {
                chunk.addOp(Opcode::AddF64);
            }
            else if (n.op == TokenType::MinusMinus) {
                chunk.addOp(Opcode::SubF64);
            }
        }
        else {
            chunk.addOp<uint8_t>(Opcode::U8, 1);

            if (n.op == TokenType::PlusPlus) {
                chunk.addOp(Opcode::AddI);
            }
            else if (n.op == TokenType::MinusMinus) {
                chunk.addOp(Opcode::SubI);
            }
        }

        if (auto var = n.node->as<VarDecl>()) {
            chunk.addOp<uint8_t>(Opcode::StoreVar, function->params.size() + (_class ? 1 : 0) + var->index);
        }
        else if (auto par = n.node->as<Param>()) {
            chunk.addOp<uint8_t>(Opcode::StoreVar, par->index);
        }
    }

    void ByteCodeCompiler::visit(UnaryExpr& n) {
        visitChild(n.target);
        switch (n.op) {
            case TokenType::Minus:
            if (n.target->type->as<IntType>()) {
                chunk.addOp<int64_t>(Opcode::I64, -1);
                chunk.addOp(Opcode::MulI);
            }
            else if (n.target->type == &FloatType::f32) {
                chunk.addOp<float>(Opcode::F32, -1);
                chunk.addOp(Opcode::MulF32);
            }
            else if (n.target->type == &FloatType::f64) {
                chunk.addOp<double>(Opcode::F64, -1);
                chunk.addOp(Opcode::MulF64);
            }
            else {
                error(n, "Invalid op");
            }
            return;
            break;

            case TokenType::ExclamationMark:
            chunk.addOp(Opcode::Not);
            return;
            break;
    
            default:
            error(n, "Unhandled unary prefix operator '" + getTokenName(n.op) + "'.");
        }
    }

    void ByteCodeCompiler::visit(EnumDecl& n) {
        visitChildren(n.elements);
    }

    void ByteCodeCompiler::visit(ThisExpr& n) {
        chunk.addOp<uint8_t>(Opcode::Var, 0);
    }
}