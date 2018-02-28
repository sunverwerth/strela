#include "Lexer.h"
#include "Parser.h"
#include "Ast/nodes.h"
#include "Types/types.h"
#include "exceptions.h"
#include "NodePrinter.h"
#include "NameResolver.h"
#include "TypeBuilder.h"
#include "TypeChecker.h"
#include "ByteCodeCompiler.h"
#include "Scope.h"
#include "VM.h"
#include "ByteCodeChunk.h"
#include "Decompiler.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace Strela;

void error(const std::string& msg) {
    std::cerr << "\033[1;31m" << msg << "\033[0m\n";
}

Scope* makeGlobalScope() {
    using namespace Types;
    auto globals = new Scope(nullptr);
    i8 = new IntType(true, 1);
    i16 = new IntType(true, 2);
    i32 = new IntType(true, 4);
    i64 = new IntType(true, 8);
    u8 = new IntType(false, 1);
    u16 = new IntType(false, 2);
    u32 = new IntType(false, 4);
    u64 = new IntType(false, 8);
    boolean = new BoolType();
    _void = new VoidType();
    f32 = new FloatType(4);
    f64 = new FloatType(8);
    null = new NullType();
    typetype = new TypeType();
    string = new ClassType("String", nullptr);
    invalid = new InvalidType();

    globals->add("i8", i8);
    globals->add("i16", i16);
    globals->add("i32", i32);
    globals->add("i64", i64);
    globals->add("u8", u8);
    globals->add("u16", u16);
    globals->add("u32", u32);
    globals->add("u64", u64);
    globals->add("bool", boolean);
    globals->add("void", _void);
    globals->add("f32", f32);
    globals->add("f64", f64);
    globals->add("null", null);
    globals->add("String", string);

    return globals;
}

#include <cstring>

namespace Strela {
    int g_timeout = -1;
}

const char* g_searchPath = nullptr;

std::string getImportFile(const std::string& baseFilename, AstImportStmt* import) {

    // First looks up file relative to the baseFilename
    // Then looks up in user lib path

    // Base: ~/MyProj/Test.strela
    // import Foo.Bar;
    // 1) ~/MyProj/Foo/Bar.strela
    // 2) ~/MyProj/Foo.strela (import member Bar)
    // 5) ~/.strela/Foo/Bar.strela
    // 6) ~/.strela/Foo.strela (import member Bar)
    
    // relative
    std::ifstream file;

    std::string relativeBase;
    auto lastSlash = baseFilename.rfind('/');
    if (lastSlash == std::string::npos) lastSlash = baseFilename.rfind('\\');
    if (lastSlash != std::string::npos) {
        relativeBase = baseFilename.substr(0, lastSlash) + "/";
    }

    std::string filename;

    filename = relativeBase + import->getFullName("/") + ".strela";
    file.open(filename, std::ios::binary);
    if (file.good()) {
        import->importModule = !import->all;
        return filename;
    }

    if (!import->all) {
        filename = relativeBase + import->getBaseName("/") + ".strela";
        file.open(filename, std::ios::binary);
        if (file.good()) {
            return filename;
        }
    }

    if (g_searchPath) {
        filename = g_searchPath + import->getFullName("/") + ".strela";
        file.open(filename, std::ios::binary);
        if (file.good()) {
            import->importModule = !import->all;
            return filename;
        }

        if (!import->all) {
            filename = g_searchPath + import->getBaseName("/") + ".strela";
            file.open(filename, std::ios::binary);
            if (file.good()) {
                return filename;
            }
        }
    }

    return "";
}

int main(int argc, char** argv) {

    std::string fileName;
    std::string byteCodePath;
    
    bool dump = false;
    bool pretty = false;
    for (int i = 1; i < argc; ++i) {
        if (i == argc - 1) {
            fileName = argv[i];
            break;
        }
        if (!strcmp(argv[i], "--dump")) dump = true;
        if (!strcmp(argv[i], "--pretty")) pretty = true;
        if (!strcmp(argv[i], "--timeout")) {
            g_timeout = std::strtol(argv[++i], nullptr, 10) * 1000;
        }
        if (!strcmp(argv[i], "--search")) {
            g_searchPath = argv[++i];
        }
        if (!strcmp(argv[i], "--write-bytecode")) {
            byteCodePath = argv[++i];
        }
    }
    
    if (fileName.empty()) {
        error("Expected file name as last cmd line argument.");
        return 1;
    }

    try {
        std::ifstream file(fileName, std::ios::binary);
        if (!file.good()) {
            error("File not found: " + fileName);
            return 1;
        }

        ByteCodeChunk chunk;

        // treat as bytecode
        bool isSourcecode = fileName.rfind(".strela") == fileName.size() - 7;

        if (isSourcecode) {
            //std::cout << "Lexing...\n";
            Lexer lexer(file);
            auto tokens = lexer.tokenize();

            //std::cout << "Parsing...\n";
            Parser parser(tokens);
            auto module = parser.parseModule();
            module->filename = fileName;

            if (pretty) {
                NodePrinter printer;
                module->accept(printer);
                return 0;
            }

            std::map<std::string, AstModDecl*> modules {
                { module->name, module },
            };

            std::vector<AstModDecl*> processImports { module };
            while (!processImports.empty()) {
                auto process = processImports.back();
                processImports.pop_back();

                for (auto&& import: process->imports) {
                    std::string importFilename(getImportFile(fileName, import));
                    if (importFilename.empty()) {
                        error(process->filename + ": Requested import not found: " + import->getFullName());
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
                        auto tokens = lexer.tokenize();
                        
                        Parser parser(tokens);
                        auto importedModule = parser.parseModule();
                        importedModule->filename = importFilename;

                        modules.insert(std::make_pair(importFilename, importedModule));
                        import->module = importedModule;

                        if (importedModule) {
                            processImports.push_back(importedModule);
                        }
                    }
                }
            }

            auto globals = makeGlobalScope();

            //std::cout << "Resolving names...\n";
            for (auto&& it: modules) {
                NameResolver resolver(globals);
                it.second->accept(resolver);
            }

            // building types
            for (auto&& it: modules) {
                TypeBuilder builder;
                it.second->accept(builder);
            }

            //std::cout << "Running type checker...\n";
            for (auto&& it: modules) {
                TypeChecker typeChecker;
                it.second->accept(typeChecker);
            }

            //std::cout << "Compiling bytecode...\n";
            ByteCodeCompiler compiler(chunk);
            module->accept(compiler);

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

        //std::cout << "Running bytecode...\n";
        VM vm(chunk);
        return vm.run().value.i64;
    }
    catch (const Exception& e) {
        error(fileName + ":" + e.what());
        return 1;
    }
}