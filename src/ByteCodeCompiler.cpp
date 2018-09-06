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

        vmtype->name = type->getFullName();
        
        vmtype->isObject = (type->as<ClassDecl>() || type->as<InterfaceDecl>() || type->as<UnionType>()) && type != &ClassDecl::String;
        vmtype->isArray = type->as<ArrayType>();
        vmtype->isEnum = type->as<EnumDecl>();

        if (vmtype->isArray) {
            vmtype->objectAlignment = 8;
            vmtype->size = 8;
            vmtype->alignment = 8;;
            vmtype->arrayType = mapType(type->as<ArrayType>()->baseType);
            vmtype->fields.push_back({
                "length",
                mapType(&IntType::u64),
                0
            });
        }
        else if (vmtype->isObject) {
            if (auto cls = type->as<ClassDecl>()) {
                vmtype->size = 8;;
                vmtype->alignment = 8;;
                size_t offset = 0;
                size_t alignment = 1;
                for (auto&& field: cls->fields) {
                    auto ftype = mapType(field->declType);
                    if (ftype->alignment > alignment) alignment = ftype->alignment;
                    offset = align(offset, alignment);
                    vmtype->fields.push_back({
                        field->name,
                        ftype,
                        offset
                    });
                    if (ftype->isArray || ftype->isObject) {
                        offset += 8;
                    }
                    else {
                        offset += ftype->size;
                    }
                }
                vmtype->objectSize = align(offset, alignment);
                vmtype->objectAlignment = alignment;
            }
            else if (auto iface = type->as<InterfaceDecl>()) {
                vmtype->size = 8;;
                vmtype->alignment = 8;;
                vmtype->objectSize = 8 + iface->methods.size() * 8;
                vmtype->objectAlignment = 8;
            }
            else if (auto un = type->as<UnionType>()) {
                vmtype->size = 8;
                vmtype->alignment = 8;
                vmtype->objectSize = 16;
                vmtype->objectAlignment = 8;
                vmtype->fields.push_back({"_tag", mapType(&IntType::u64), 0});
                vmtype->fields.push_back({"_ref", mapType(&IntType::u64), 8});
            }
        }
        else if (auto intt = type->as<IntType>()) {
            vmtype->size = intt->bytes;
            vmtype->alignment = intt->bytes;
        }
        else if (auto en = type->as<EnumDecl>()) {
            vmtype->size = 4;
            vmtype->alignment = 4;
            vmtype->isEnum = true;
            for (auto& ee: en->elements) {
                vmtype->enumValues.push_back(ee->name);
            }
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
            vmtype->size = 8;
            vmtype->alignment = 8;
        }
        else if (type == &PointerType::instance) {
            vmtype->size = 8;
            vmtype->alignment = 8;
        }
        
        return vmtype;
    }

    ByteCodeCompiler::ByteCodeCompiler(ByteCodeChunk& chunk): chunk(chunk) {
    }

    void ByteCodeCompiler::addFixup(size_t address, FuncDecl* function, bool immediate) {
        functionFixups.push_back({address, function, immediate});
    }

    void ByteCodeCompiler::visit(ModDecl& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChildren(n.functions);
        visitChildren(n.classes);

        // fixup function pointers
        while (!functionFixups.empty()) {
            auto fixup = functionFixups.back();
            functionFixups.pop_back();

            if (fixup.function->opcodeStart == 0xdeadbeef) {
                _class = fixup.function->parent ? fixup.function->parent->as<ClassDecl>() : nullptr;
                fixup.function->accept(*this);
            }

            if (fixup.immediate) {
                chunk.write(fixup.address + 1, &fixup.function->opcodeStart, sizeof(uint32_t));
            }
            else {
                auto index = chunk.addConstant(VMValue(int64_t(fixup.function->opcodeStart)));
                chunk.writeArgument(fixup.address, index);
            }
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
        if (n.source) chunk.setLine(n.source->filename, n.line);
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

    FunctionInfo* fi;

    void ByteCodeCompiler::visit(FuncDecl& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        auto oldfunc = function;
        function = &n;

        ClassDecl* cls = n.parent ? n.parent->as<ClassDecl>() : nullptr;
        n.opcodeStart = chunk.opcodes.size();
        std::stringstream sstr;
        if (cls) {
            sstr << cls->getFullName() << ".";
        }
        sstr << n.name << n.declType->getFullName();

        FunctionInfo funcInfo;
        fi = &funcInfo;
        funcInfo.name = sstr.str();

        if (cls) {
            funcInfo.variables.push_back({ 0, "this", mapType(cls) });
        }
        for (size_t i = 0; i < n.params.size(); ++i) {
            n.params[i]->index = cls ? i + 1 : i;
            funcInfo.variables.push_back({ n.params[i]->index, n.params[i]->name, mapType(n.params[i]->declType) });
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
			if (n.source) chunk.setLine(n.source->filename, n.lineend);
            chunk.addOp(Opcode::ReturnVoid);
        }

        chunk.addFunction(n.opcodeStart, funcInfo);
        function = oldfunc;
    }

    void ByteCodeCompiler::visit(VarDecl& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        mapType(n.declType);
        if (n.initializer) {
            n.initializer->accept(*this);
            chunk.addOp<uint8_t>(Opcode::StoreVar, function->params.size() + (_class ? 1 : 0) + n.index);
        }
        fi->variables.push_back({ int(function->params.size() + (_class ? 1 : 0) + n.index), n.name, mapType(n.declType) });
    }

    void ByteCodeCompiler::visit(FieldDecl& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        mapType(n.declType);
    }

    void ByteCodeCompiler::visit(Param& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        mapType(n.declType);
    }

    void ByteCodeCompiler::visit(IdExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        if (auto fun = n.node->as<FuncDecl>()) {
            if (fun->isExternal) {
                auto index = chunk.addForeignFunction(*fun);
                chunk.addOp<uint64_t>(Opcode::I64, index);
            }
            else {
                if (n.context) {
                    visitChild(n.context);
                }
                auto index = chunk.addOp<uint16_t>(Opcode::Const, 255);
                addFixup(index, fun, false);
            }
        }
        else if (auto param = n.node->as<Param>()) {
            chunk.addOp<uint8_t>(Opcode::Var, param->index);
        }
        else if (auto var = n.node->as<VarDecl>()) {
            chunk.addOp<uint8_t>(Opcode::Var, function->params.size() + (_class ? 1 : 0) + var->index);
        }
        else if (auto field = n.node->as<FieldDecl>()) {
            auto t = mapType(n.context->type);
            auto ft = mapType(field->declType);
            Opcode op;
            switch (ft->size) {
                case 1: op = Opcode::Ptr8; break;
                case 2: op = Opcode::Ptr16; break;
                case 4: op = Opcode::Ptr32; break;
                case 8: op = Opcode::Ptr64; break;
            }
            if (op == Opcode::Ptr64) {
                chunk.addOp<uint8_t, uint8_t>((ft->isObject || ft->isArray) ? Opcode::ObjPtr64Var : Opcode::Ptr64Var, t->fields[field->index].offset, 0);
            }
            else {
                visitChild(n.context);
                chunk.addOp<uint8_t>(op, t->fields[field->index].offset);
            }
        }
    }

    void ByteCodeCompiler::visit(ExprStmt& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChild(n.expression);
        if (n.expression->type != &VoidType::instance && !n.expression->as<AssignExpr>() && !n.expression->as<PostfixExpr>()) {
            chunk.addOp(Opcode::Pop);
        }
    }

    void ByteCodeCompiler::visit(CallExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        if (n.callTarget->node && n.callTarget->node->as<FuncDecl>() && n.callTarget->context) {
            visitChild(n.callTarget->context);
        }

        visitChildren(n.arguments);

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
            if (n.callTarget->node && n.callTarget->node->as<FuncDecl>() && n.callTarget->node->as<FuncDecl>()->isExternal) {
                visitChild(n.callTarget);
                chunk.addOp(Opcode::NativeCall);
            }
            else {
                if (n.callTarget->node && n.callTarget->node->as<FuncDecl>()) {
                    auto ind = chunk.addOp<uint32_t, uint8_t>(Opcode::CallImm, 0xffffffff, n.callTarget->type->as<FuncType>()->paramTypes.size() + (n.callTarget->context ? 1 : 0));
                    addFixup(ind, n.callTarget->node->as<FuncDecl>(), true);
                }
                else {
                    visitChild(n.callTarget);
                    chunk.addOp<uint8_t>(Opcode::Call, n.callTarget->type->as<FuncType>()->paramTypes.size() + (n.callTarget->context ? 1 : 0));
                }
            }
        }
    }

    void ByteCodeCompiler::visit(RetStmt& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        if (n.expression) {
            visitChild(n.expression);
            chunk.addOp(Opcode::Return);
        }
        else {
            chunk.addOp(Opcode::ReturnVoid);
        }
    }

    void ByteCodeCompiler::visit(LitExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        int index = 0;
        if (auto intt = n.type->as<IntType>()) {
            auto val = n.token.intVal();
            if (val < 0) {
                if (val < 0x80000000) {
                    chunk.addOp<int64_t>(Opcode::I64, val);
                }
                else if (val < 0x8000) {
                    chunk.addOp<int32_t>(Opcode::I32, val);
                }
                else if (val < 0x80) {
                    chunk.addOp<int16_t>(Opcode::I16, val);
                }
                else {
                    chunk.addOp<int8_t>(Opcode::I8, val);
                }
            }
            else {
                if (val > 0xffffffff) {
                    chunk.addOp<uint64_t>(Opcode::U64, val);
                }
                else if (val > 0xffff) {
                    chunk.addOp<uint32_t>(Opcode::U32, val);
                }
                else if (val > 0xff) {
                    chunk.addOp<uint16_t>(Opcode::U16, val);
                }
                else {
                    chunk.addOp<uint8_t>(Opcode::U8, val);
                }
            }
        }
        else if (n.type == &FloatType::f32) {
            chunk.addOp<float>(Opcode::F32, n.token.floatVal());
        }
        else if (n.type == &FloatType::f64) {
            chunk.addOp<uint16_t>(Opcode::Const, chunk.addConstant(VMValue(n.token.floatVal())));
        }
        else if (n.type == &BoolType::instance) {
            chunk.addOp<uint8_t>(Opcode::U8, (n.token.boolVal() ? 1 : 0));
        }
        else if (n.type == &ClassDecl::String) {
            index = chunk.addConstant(VMValue(n.token.value.c_str()));
            chunk.addOp<uint16_t>(Opcode::Const, index);
        }
        else if (n.type == &NullType::instance) {
            chunk.addOp(Opcode::Null);
        }
        else error(n, "Invalid literal type");
    }

    void ByteCodeCompiler::visit(IsExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChild(n.target);
        chunk.addOp<uint8_t>(Opcode::Ptr64, 0);
        chunk.addOp<int64_t>(Opcode::I64, n.typeTag);
        chunk.addOp(Opcode::CmpEQ);
    }

    void ByteCodeCompiler::visit(CastExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
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
            chunk.addOp<uint16_t>(Opcode::New, mapType(toiface)->index);
            visitChild(n.sourceExpr);
            chunk.addOp<uint8_t>(Opcode::Peek, 1);
            chunk.addOp<uint8_t>(Opcode::StorePtr64, 0);
            for(size_t i = 0; i < toiface->methods.size(); ++i) {
                auto index = chunk.addOp<uint16_t>(Opcode::Const, 255);
                addFixup(index, n.implementation->classMethods[i], false);
                chunk.addOp<uint8_t>(Opcode::Peek, 1);
                chunk.addOp<uint8_t>(Opcode::StorePtr64, i * 8 + 8);
            }
        }
        else if (tounion) {
            auto tag = tounion->getTypeTag(fromtype);
            chunk.addOp<uint16_t>(Opcode::New, mapType(tounion)->index);
            
            chunk.addOp(Opcode::Repeat);
            chunk.addOp<int64_t>(Opcode::U64, tag);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(Opcode::StorePtr64, 0);

            chunk.addOp(Opcode::Repeat);
            visitChild(n.sourceExpr);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(Opcode::StorePtr64, 8);
        }
        else if (fromunion) {
            visitChild(n.sourceExpr);
            chunk.addOp<uint8_t>(Opcode::ObjPtr64, 8);
        }
        else if (fromtype == &FloatType::f32 && totype->as<IntType>()) {
            if (auto lit = n.sourceExpr->as<LitExpr>()) {
                chunk.addOp<int64_t>(Opcode::I64, lit->token.floatVal());
            }
            else {
                visitChild(n.sourceExpr);
                chunk.addOp(Opcode::F32tI64);
            }
        }
        else if (fromtype == &FloatType::f64 && totype->as<IntType>()) {
            if (auto lit = n.sourceExpr->as<LitExpr>()) {
                chunk.addOp<int64_t>(Opcode::I64, lit->token.floatVal());
            }
            else {
                visitChild(n.sourceExpr);
                chunk.addOp(Opcode::F64tI64);
            }
        }
        else if (fromtype->as<IntType>() && totype == &FloatType::f32) {
            if (auto lit = n.sourceExpr->as<LitExpr>()) {
                chunk.addOp<float>(Opcode::F32, lit->token.intVal());
            }
            else {
                visitChild(n.sourceExpr);
                chunk.addOp(Opcode::I64tF32);
            }
        }
        else if (fromtype->as<IntType>() && totype == &FloatType::f64) {
            if (auto lit = n.sourceExpr->as<LitExpr>()) {
                chunk.addOp<uint16_t>(Opcode::Const, chunk.addConstant(VMValue((double)lit->token.intVal())));
            }
            else {
                visitChild(n.sourceExpr);
                chunk.addOp(Opcode::I64tF64);
            }
        }
        else if (fromtype == &FloatType::f32 && totype == &FloatType::f64) {
            if (auto lit = n.sourceExpr->as<LitExpr>()) {
                chunk.addOp<uint16_t>(Opcode::Const, chunk.addConstant(VMValue((double)lit->token.floatVal())));
            }
            else {
                visitChild(n.sourceExpr);
                chunk.addOp(Opcode::F32tF64);
            }
        }
        else if (fromtype == &FloatType::f64 && totype == &FloatType::f32) {
            if (auto lit = n.sourceExpr->as<LitExpr>()) {
                chunk.addOp<float>(Opcode::F32, lit->token.floatVal());
            }
            else {
                visitChild(n.sourceExpr);
                chunk.addOp(Opcode::F64tF32);
            }
        }
        else {
            visitChild(n.sourceExpr);
        }
    }

    void ByteCodeCompiler::visit(BlockStmt& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChildren(n.stmts);
		if (n.source) chunk.setLine(n.source->filename, n.lineend);
	}

    void ByteCodeCompiler::visit(BinopExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        if (n.function) {
            visitChild(n.left);
            visitChild(n.right);
            auto index = chunk.addOp<uint16_t>(Opcode::Const, 255);
            addFixup(index, n.function, false);
            chunk.addOp<uint8_t>(Opcode::Call, 2);
            return;
        }

        if (n.op == TokenType::AmpAmp) {
            visitChild(n.left);
            auto const1 = chunk.addOp<uint16_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::JmpIfNot);
            chunk.addOp<uint8_t>(Opcode::U8, 1);
            visitChild(n.right);
            chunk.addOp(Opcode::AndL);
            auto const2 = chunk.addOp<uint16_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::Jmp);
            chunk.writeArgument(const1, chunk.addConstant(VMValue((int64_t)chunk.opcodes.size())));
            chunk.addOp<uint8_t>(Opcode::U8, 0);
            chunk.writeArgument(const2, chunk.addConstant(VMValue((int64_t)chunk.opcodes.size())));            
        }
        else if (n.op == TokenType::PipePipe) {
            visitChild(n.left);
            auto const1 = chunk.addOp<uint16_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::JmpIf);
            chunk.addOp<uint8_t>(Opcode::U8, 0);
            visitChild(n.right);
            chunk.addOp(Opcode::OrL);
            auto const2 = chunk.addOp<uint16_t>(Opcode::Const, 0);
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
                    error(n, "Plus: Invalid op");
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
                    error(n, "Minus: Invalid op");
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
                    error(n, "Mul: Invalid op");
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
                    error(n, "Div: Invalid op");
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
        if (n.source) chunk.setLine(n.source->filename, n.line);
        if (auto fun = n.node->as<FuncDecl>()) {
            visitChild(n.scopeTarget);
            if (fun->isExternal) {
                auto index = chunk.addForeignFunction(*fun);
                chunk.addOp<uint64_t>(Opcode::I64, index);
            }
            else {
                auto index = chunk.addOp<uint16_t>(Opcode::Const, 255);
                addFixup(index, fun, false);
            }
        }
        else if (auto im = n.node->as<InterfaceMethodDecl>()) {
            visitChild(n.scopeTarget);
            chunk.addOp(Opcode::Repeat);
            chunk.addOp<uint8_t>(Opcode::Ptr64, 8 + im->index * 8);
            chunk.addOp(Opcode::Swap);
            chunk.addOp<uint8_t>(Opcode::ObjPtr64, 0);
            chunk.addOp(Opcode::Swap);
        }
        else if (auto field = n.node->as<FieldDecl>()) {
            auto t = mapType(n.scopeTarget->type);
            auto ft = mapType(field->declType);
            Opcode op;
            switch (ft->size) {
                case 1: op = Opcode::Ptr8; break;
                case 2: op = Opcode::Ptr16; break;
                case 4: op = Opcode::Ptr32; break;
                case 8: op = Opcode::Ptr64; break;
            }

            if (op == Opcode::Ptr64 && n.scopeTarget->as<IdExpr>() && n.scopeTarget->as<IdExpr>()->node->as<VarDecl>()) {
                auto var = n.scopeTarget->as<IdExpr>()->node->as<VarDecl>();
                chunk.addOp<uint8_t, uint8_t>((ft->isObject || ft->isArray) ? Opcode::ObjPtr64Var : Opcode::Ptr64Var, t->fields[field->index].offset, function->params.size() + (_class ? 1 : 0) + var->index);
            }
            else if (op == Opcode::Ptr64 && n.scopeTarget->as<IdExpr>() && n.scopeTarget->as<IdExpr>()->node->as<Param>()) {
                auto par = n.scopeTarget->as<IdExpr>()->node->as<Param>();
                chunk.addOp<uint8_t, uint8_t>((ft->isObject || ft->isArray) ? Opcode::ObjPtr64Var : Opcode::Ptr64Var, t->fields[field->index].offset, par->index);
            }
            else if (op == Opcode::Ptr64 && n.scopeTarget->as<ThisExpr>()) {
                chunk.addOp<uint8_t, uint8_t>((ft->isObject || ft->isArray) ? Opcode::ObjPtr64Var : Opcode::Ptr64Var, t->fields[field->index].offset, 0);
            }
            else {
                visitChild(n.scopeTarget);
                chunk.addOp<uint8_t>(op, t->fields[field->index].offset);
            }
        }
        else if (auto ee = n.node->as<EnumElement>()) {
            chunk.addOp<int64_t>(Opcode::I64, ee->index);
        }
        else {
            error(n, "Invalid scope expr");
        }
    }

    void ByteCodeCompiler::visit(ArrayLitExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        chunk.addOp<uint64_t>(Opcode::U64, mapType(n.type)->index);
        chunk.addOp<uint64_t>(Opcode::U64, n.elements.size());
        chunk.addOp(Opcode::Array);
        auto t = mapType(n.type->as<ArrayType>()->baseType);
        Opcode op;
        switch (t->size) {
            case 1: op = Opcode::StorePtr8; break;
            case 2: op = Opcode::StorePtr16; break;
            case 4: op = Opcode::StorePtr32; break;
            case 8: op = Opcode::StorePtr64; break;
        }
        size_t i = 8;
        for (auto&& el: n.elements) {
            visitChild(el);
            chunk.addOp<uint8_t>(Opcode::Peek, 1);
            chunk.addOp<uint8_t>(op, i);
            i += t->size;
        }
    }

    void ByteCodeCompiler::visit(SubscriptExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        if (n.subscriptFunction) {
            visitChild(n.callTarget);
            visitChildren(n.arguments);
            auto index = chunk.addOp<uint16_t>(Opcode::Const, 255);
            addFixup(index, n.subscriptFunction, false);
            chunk.addOp<uint8_t>(Opcode::Call, n.arguments.size() + 1);
        }
        else {
            auto fieldSize = mapType(n.callTarget->type)->arrayType->size;
            auto ft = mapType(n.callTarget->type->as<ArrayType>()->baseType);
            for (int i = n.arguments.size() - 1; i >= 0; --i) {
                visitChild(n.arguments[i]);
                if (fieldSize != 1) {
                    chunk.addOp<uint8_t>(Opcode::U8, fieldSize);
                    chunk.addOp(Opcode::MulI);
                }
            }
            Opcode op;
            switch (fieldSize) {
                case 1: op = Opcode::PtrInd8; break;
                case 2: op = Opcode::PtrInd16; break;
                case 4: op = Opcode::PtrInd32; break;
                case 8: op = Opcode::PtrInd64; break;
            }

            visitChild(n.callTarget);
            chunk.addOp<uint8_t>((ft->isObject || ft->isArray) ? Opcode::ObjPtrInd64 : op, 8);
        }
    }

    void ByteCodeCompiler::visit(IfStmt& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChild(n.condition);
        auto pos = chunk.addOp<uint16_t>(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.trueBranch);

        int pos2;
        if (n.falseBranch) {
            pos2 = chunk.addOp<uint16_t>(Opcode::Const, 0);
            chunk.addOp(Opcode::Jmp);
        }

        auto index = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos, index);
        
        if (n.falseBranch) {
            visitChild(n.falseBranch);
            auto index2 = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
            chunk.writeArgument(pos2, index2);
        }
    }

    void ByteCodeCompiler::visit(NewExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        if (auto clstype = n.type->as<ClassDecl>()) {
            chunk.addOp<uint16_t>(Opcode::New, mapType(clstype)->index);
            if (n.initMethod) {
                chunk.addOp(Opcode::Repeat);
                visitChildren(n.arguments);
                auto ind = chunk.addOp<uint32_t, uint8_t>(Opcode::CallImm, 0xffffffff, n.arguments.size() + 1);
                addFixup(ind, n.initMethod, true);
            }
        }
        else if (auto arrtype = n.type->as<ArrayType>()) {
            chunk.addOp<uint64_t>(Opcode::U64, mapType(arrtype)->index);
            visitChild(n.arguments.front());
            chunk.addOp(Opcode::Array);
        }
    }

    void ByteCodeCompiler::visit(AssignExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChild(n.right);
        if (!n.parent->as<ExprStmt>()) {
            chunk.addOp(Opcode::Repeat);
        }

        if (n.left->arrayIndex) {
            auto fieldSize = mapType(n.left->context->type)->arrayType->size;
            visitChild(n.left->arrayIndex);
            if (fieldSize != 1) {
                chunk.addOp<uint8_t>(Opcode::U8, fieldSize);
                chunk.addOp(Opcode::MulI);
            }
            Opcode op;
            switch (fieldSize) {
                case 1: op = Opcode::StorePtrInd8; break;
                case 2: op = Opcode::StorePtrInd16; break;
                case 4: op = Opcode::StorePtrInd32; break;
                case 8: op = Opcode::StorePtrInd64; break;
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
            auto t = mapType(n.left->context->type);
            auto ft = mapType(field->declType);
            Opcode op;
            switch (ft->size) {
                case 1: op = Opcode::StorePtr8; break;
                case 2: op = Opcode::StorePtr16; break;
                case 4: op = Opcode::StorePtr32; break;
                case 8: op = Opcode::StorePtr64; break;
            }

            if (op == Opcode::StorePtr64 && n.left->context->as<IdExpr>() && n.left->context->as<IdExpr>()->node->as<VarDecl>()) {
                auto var = n.left->context->as<IdExpr>()->node->as<VarDecl>();
                chunk.addOp<uint8_t, uint8_t>(Opcode::StorePtr64Var, t->fields[field->index].offset, function->params.size() + (_class ? 1 : 0) + var->index);
            }
            else if (op == Opcode::StorePtr64 && n.left->context->as<IdExpr>() && n.left->context->as<IdExpr>()->node->as<Param>()) {
                auto par = n.left->context->as<IdExpr>()->node->as<Param>();
                chunk.addOp<uint8_t, uint8_t>(Opcode::StorePtr64Var, t->fields[field->index].offset, par->index);
            }
            else if (op == Opcode::StorePtr64 && n.left->context->as<ThisExpr>()) {
                chunk.addOp<uint8_t, uint8_t>(Opcode::StorePtr64Var, t->fields[field->index].offset, 0);
            }
            else {
                visitChild(n.left->context);
                chunk.addOp<uint8_t>(op, t->fields[field->index].offset);
            }
        }
    }

    void ByteCodeCompiler::visit(WhileStmt& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        auto startPos = chunk.opcodes.size();
        visitChild(n.condition);
        auto pos = chunk.addOp<uint16_t>(Opcode::Const, 0);
        chunk.addOp(Opcode::JmpIfNot);
        visitChild(n.body);
        chunk.addOp<uint16_t>(Opcode::Const, chunk.addConstant(VMValue(int64_t(startPos))));
        chunk.addOp(Opcode::Jmp);
        auto index = chunk.addConstant(VMValue(int64_t(chunk.opcodes.size())));
        chunk.writeArgument(pos, index);
    }

    void ByteCodeCompiler::visit(PostfixExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChild(n.target);
        if (!n.parent->as<ExprStmt>()) {
            chunk.addOp(Opcode::Repeat);
        }

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
            chunk.addOp<uint16_t>(Opcode::Const, chunk.addConstant(VMValue((double)1)));

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
        if (n.source) chunk.setLine(n.source->filename, n.line);
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
                chunk.addOp<uint16_t>(Opcode::Const, chunk.addConstant(VMValue((double)-1)));
                chunk.addOp(Opcode::MulF64);
            }
            else {
                error(n, "UnaryExpr: Invalid op");
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
        if (n.source) chunk.setLine(n.source->filename, n.line);
        visitChildren(n.elements);
    }

    void ByteCodeCompiler::visit(ThisExpr& n) {
        if (n.source) chunk.setLine(n.source->filename, n.line);
        chunk.addOp<uint8_t>(Opcode::Var, 0);
    }
}