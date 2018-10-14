// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "Lexer.h"
#include "Parser.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "NodePrinter.h"
#include "NameResolver.h"
#include "TypeChecker.h"
#include "ByteCodeCompiler.h"
#include "Scope.h"
#include "VM/VM.h"
#include "VM/ByteCodeChunk.h"
#include "Decompiler.h"
#include "SourceFile.h"
#include "VM/Debugger.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstring>

using namespace Strela;

namespace Strela {
	int g_timeout = -1;
	std::string g_homePath;
	std::string g_searchPath;
	unsigned short g_debugPort = 0;
}

void error(const std::string& msg) {
    std::cerr << "\033[1;31m" << msg << "\033[0m\n";
}

void error(const Strela::Node* n, const std::string& msg) {
    std::cerr << "\033[1;31m" << n->source->filename << ":" << n->line << ":" << n->column << " " << msg << "\033[0m\n";
}

void bail() {
    error("Aborting due to previous errors.");
    exit(1);
}

void help() {
    std::cout << "strela - The strela compiler and VM\n";
    std::cout << "general usage:\n";
    std::cout << "strela [options] input-file\n";
    std::cout << "\n";
    std::cout << "options are:\n";
    std::cout << "    --dump             dumps decompiled bytecode to stdout and exits.\n";
    std::cout << "    --pretty           pretty-prints the parsed code to stdout and exits.\n";
    std::cout << "    --timeout <sec>    kills the running program after <sec> seconds.\n";
    std::cout << "    --search <path>    sets additional search path <path> for imports.\n";
    std::cout << "    --write-bytecode <file>    writes compiled bytecode to <file> and exits.\n";
}

Scope* makeGlobalScope() {
    auto globals = new Scope(nullptr);

    globals->add("int", &IntType::i64);
    globals->add("u8", &IntType::u8);
    globals->add("u16", &IntType::u16);
    globals->add("u32", &IntType::u32);
    globals->add("u64", &IntType::u64);
    globals->add("i8", &IntType::i8);
    globals->add("i16", &IntType::i16);
    globals->add("i32", &IntType::i32);
    globals->add("i64", &IntType::i64);
    globals->add("bool", &BoolType::instance);
    globals->add("void", &VoidType::instance);
    globals->add("float", &FloatType::f64);
    globals->add("f32", &FloatType::f32);
    globals->add("f64", &FloatType::f64);
    globals->add("null", &NullType::instance);
    globals->add("Ptr", &PointerType::instance);

    std::string fileName;
    if (g_searchPath.size() && std::ifstream(Strela::g_searchPath + "/Std/core.strela")) {
        fileName = Strela::g_searchPath + "/Std/core.strela";
    }
    else if (std::ifstream(Strela::g_homePath + "/.strela/lib/Std/core.strela")) {
        fileName = Strela::g_homePath + "/.strela/lib/Std/core.strela";
    }
    else if (std::ifstream("/usr/local/lib/strela/Std/core.strela")) {
        fileName = "/usr/local/lib/strela/Std/core.strela";
    }

    if (fileName.empty()) {
        error("Unable to locate core library.");
        bail();
    }

    std::ifstream sourceFile(fileName, std::ios::binary);
    Lexer lexer(sourceFile);
    auto source = new SourceFile(fileName, lexer.tokenize());
    if (lexer.hadErrors()) bail();
    Parser parser(*source);
    auto module = parser.parseModDecl();
    if (parser.hadErrors()) bail();

    module->_name = "";
    module->filename = fileName;

    ClassDecl::String = module->getClass("String");

    NameResolver resolver(globals);
    resolver.resolve(*module);

    TypeChecker checker;
    checker.check(*module);

    for (auto& cls: module->classes) {
        if (cls->isExported) {
            globals->add(cls->_name, cls);
        }
    }

    for (auto& fun: module->functions) {
        if (fun->isExported) {
            globals->add(fun->name, fun);
        }
    }

    return globals;
}

std::string normalizePath(const std::string& path) {
    if (path.empty()) return "./";
    if (path.back() != '/' && path.back() != '\\') {
        return path + "/";
    }
    return path;
}

std::string getImportFile(const std::string& baseFilename, ImportStmt* import) {

    // First looks up file relative to the baseFilename
    // Then looks up in user lib path

    // Base: ~/MyProj/Test.strela
    // import Foo.Bar;
    // 1) ~/MyProj/Foo/Bar.strela
    // 2) ~/MyProj/Foo.strela (import member Bar)
    // 3) <searchpath>/Foo/Bar.strela
    // 4) <searchpath>/Foo.strela (import member Bar)
    // 5) <homepath>/.strela/Foo/Bar.strela
    // 6) <homepath>/.strela/Foo.strela (import member Bar)
    // 7) /usr/local/lib/strela/Foo/Bar.strela
    // 8) /usr/local/lib/strela/Foo.strela (import member Bar)
    
    std::ifstream file;

    std::string relativeBase("./");
    auto lastSlash = baseFilename.rfind('/');
    if (lastSlash == std::string::npos) lastSlash = baseFilename.rfind('\\');
    if (lastSlash != std::string::npos) {
        relativeBase = baseFilename.substr(0, lastSlash) + "/";
    }

    // local
    std::vector<std::pair<std::string, bool>> tries;
    tries.push_back({relativeBase + import->getFullName("/") + ".strela", true});
    if (!import->all) tries.push_back({relativeBase + import->getBaseName("/") + ".strela", false});

    // additional
    if (g_searchPath.size()) {
        tries.push_back({g_searchPath + import->getFullName("/") + ".strela", true});
        if (!import->all) tries.push_back({g_searchPath + import->getBaseName("/") + ".strela", false});
    }

    // home
    tries.push_back({Strela::g_homePath + ".strela/lib/" + import->getFullName("/") + ".strela", true});
    if (!import->all) tries.push_back({ Strela::g_homePath + ".strela/lib/" + import->getBaseName("/") + ".strela", false});
    
    // global
    tries.push_back({"/usr/local/lib/strela/" + import->getFullName("/") + ".strela", true});
    if (!import->all) tries.push_back({"/usr/local/lib/strela/" + import->getBaseName("/") + ".strela", false});
    
    for (auto&& it: tries) {
        file.open(it.first, std::ios::binary);
        if (file.good()) {
            import->importModule = it.second && !import->all;
            return it.first;
        }
    }

    return "";
}

int main(int argc, char** argv) {

    #ifdef _WIN32
    g_homePath = std::string(getenv("HOMEDRIVE")) + getenv("HOMEPATH");
    #else
    g_homePath = getenv("HOME");
    #endif
    g_homePath = normalizePath(g_homePath);

    std::string fileName;
    std::string byteCodePath;
    std::vector<std::string> arguments;
    
    bool dump = false;
    bool pretty = false;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--dump")) dump = true;
        else if (!strcmp(argv[i], "--pretty")) pretty = true;
        else if (!strcmp(argv[i], "--timeout")) {
            g_timeout = std::strtol(argv[++i], nullptr, 10) * 1000;
        }
        else if (!strcmp(argv[i], "--search")) {
            g_searchPath = argv[++i];
            g_searchPath = normalizePath(g_searchPath);
        }
        else if (!strcmp(argv[i], "--write-bytecode")) {
            byteCodePath = argv[++i];
        }
        else if (!strcmp(argv[i], "--debug")) {
            g_debugPort = std::strtoul(argv[++i], nullptr, 10);
        }
        else {
            fileName = argv[i++];
            for (int j = i; j < argc; ++j) {
                arguments.push_back(argv[j]);
            }
            break;
        }
    }
    
    if (fileName.empty()) {
        error("Expected file name as last cmd line argument.");
        help();
        return 1;
    }

    try {
        bool errors = false;
        std::ifstream file(fileName, std::ios::binary);
        if (!file.good()) {
            error("File not found: " + fileName);
            return 1;
        }

        ByteCodeChunk chunk;

        // treat as bytecode
        bool isSourcecode = fileName.rfind(".strela") != std::string::npos;

        if (isSourcecode) {
            //std::cout << "Lexing...\n";
            Lexer lexer(file);
            auto source = new SourceFile(fileName, lexer.tokenize());
            if (lexer.hadErrors()) bail();

            //std::cout << "Parsing...\n";
            Parser parser(*source);
            auto module = parser.parseModDecl();
            if (parser.hadErrors()) bail();
            module->filename = fileName;

            if (pretty) {
                NodePrinter printer;
                printer.print(*module);
                return 0;
            }

            auto globals = makeGlobalScope();

            std::map<std::string, ModDecl*> modules {
                { module->getFullName(), module },
            };

            std::vector<ModDecl*> processImports { module };
            while (!processImports.empty()) {
                auto process = processImports.back();
                processImports.pop_back();

                for (auto&& import: process->imports) {
                    std::string importFilename(getImportFile(process->filename, import));
                    if (importFilename.empty()) {
                        error(import, "Requested import not found: " + import->getFullName());
                        return 1;
                    }

                    auto it = modules.find(importFilename);
                    if (it != modules.end()) {
                        import->module = it->second;
                    }
                    else {
                        std::ifstream importFile(importFilename, std::ios::binary);
                        if (!importFile.good()) {
                            error("Imported file is not readable: " + importFilename);
                            return 1;
                        }

                        Lexer lexer(importFile);
                        auto source = new SourceFile(importFilename, lexer.tokenize());
                        
                        Parser parser(*source);
                        auto importedModule = parser.parseModDecl();
                        if (parser.hadErrors()) bail();
                        importedModule->filename = importFilename;

                        modules.insert(std::make_pair(importFilename, importedModule));
                        import->module = importedModule;

                        if (importedModule) {
                            processImports.push_back(importedModule);
                        }
                    }
                }
            }

            //std::cout << "Resolving names...\n";
            for (auto&& it: modules) {
                NameResolver resolver(globals);
                resolver.resolve(*it.second);
                errors |= resolver.hadErrors();
            }

            if (errors) bail();

            //std::cout << "Resolving generics...\n";
            int numGenerics = 0;
            do {
                numGenerics = 0;
                for (auto&& it: modules) {
                    NameResolver resolver(globals);
                    numGenerics += resolver.resolveGenerics(*it.second);
                    errors |= resolver.hadErrors();
                }
            } while (numGenerics > 0);

            if (errors) bail();

            //std::cout << "Running type checker...\n";
            for (auto&& it: modules) {
                TypeChecker typeChecker;
                typeChecker.check(*it.second);
                errors |= typeChecker.hadErrors();
            }
            
            if (errors) bail();

            //std::cout << "Compiling bytecode...\n";
            ByteCodeCompiler compiler(chunk);
            compiler.compile(*module);
            if (compiler.hadErrors()) bail();

            if (!byteCodePath.empty()) {
                std::ofstream outbin(byteCodePath, std::ios::binary);
                outbin << chunk;
                outbin.close();
                return 0;
            }
        }

        if (!isSourcecode) {
            std::ifstream inbin(fileName, std::ios::binary);
            inbin >> chunk;
            inbin.close();
        }
        
        if (dump) {
            Decompiler decompiler(chunk);
            decompiler.listing();
            return 0;
        }

		VM vm(chunk, arguments);
        if (g_debugPort > 0) {
            Debugger dbg(g_debugPort, vm);
			return dbg.run();
        }
		else {
			return vm.run();
		}
    }
    catch (const Exception& e) {
        error(e.what());
        return 1;
    }
}