#include "Lexer.h"
#include "Parser.h"
#include "Ast/nodes.h"
#include "exceptions.h"
#include "NodePrinter.h"
#include "NameResolver.h"
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

void help() {
    std::cout << "strelac - The strela compiler and VM\n";
    std::cout << "general usage:\n";
    std::cout << "strelac [options] input-file\n";
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

    globals->add("i8", &IntType::i8);
    globals->add("i16", &IntType::i16);
    globals->add("i32", &IntType::i32);
    globals->add("i64", &IntType::i64);
    globals->add("u8", &IntType::u8);
    globals->add("u16", &IntType::u16);
    globals->add("u32", &IntType::u32);
    globals->add("u64", &IntType::u64);
    globals->add("bool", &BoolType::instance);
    globals->add("void", &VoidType::instance);
    globals->add("f32", &FloatType::f32);
    globals->add("f64", &FloatType::f64);
    globals->add("null", &NullType::instance);
    globals->add("String", &ClassDecl::String);

    return globals;
}

#include <cstring>

namespace Strela {
    int g_timeout = -1;
    std::string g_homePath;
    std::string g_searchPath;
}


std::string getImportFile(const std::string& baseFilename, ImportStmt* import) {

    // First looks up file relative to the baseFilename
    // Then looks up in user lib path

    // Base: ~/MyProj/Test.strela
    // import Foo.Bar;
    // 1) ~/MyProj/Foo/Bar.strela
    // 2) ~/MyProj/Foo.strela (import member Bar)
    // 3) <homepath>/.strela/Foo/Bar.strela
    // 4) <homepath>/.strela/Foo.strela (import member Bar)
    // 5) /usr/local/lib/strela/Foo/Bar.strela
    // 6) /usr/local/lib/strela/Foo.strela (import member Bar)
    // 7) <searchpath>/Foo/Bar.strela
    // 8) <searchpath>/Foo.strela (import member Bar)
    
    std::ifstream file;

    std::string relativeBase;
    auto lastSlash = baseFilename.rfind('/');
    if (lastSlash == std::string::npos) lastSlash = baseFilename.rfind('\\');
    if (lastSlash != std::string::npos) {
        relativeBase = baseFilename.substr(0, lastSlash) + "/";
    }

    // local
    std::vector<std::pair<std::string, bool>> tries;
    tries.push_back({relativeBase + import->getFullName("/") + ".strela", true});
    if (!import->all) tries.push_back({relativeBase + import->getBaseName("/") + ".strela", false});

    // home
    tries.push_back({Strela::g_homePath + "/.strela/" + import->getFullName("/") + ".strela", true});
    if (!import->all) tries.push_back({ Strela::g_homePath + "/.strela/" + import->getBaseName("/") + ".strela", false});
    
    // global
    tries.push_back({"/usr/local/lib/strela/" + import->getFullName("/") + ".strela", true});
    if (!import->all) tries.push_back({"/usr/local/lib/strela/" + import->getBaseName("/") + ".strela", false});
    
    // additional
    if (g_searchPath.size()) {
        tries.push_back({g_searchPath + import->getFullName("/") + ".strela", true});
        if (!import->all) tries.push_back({g_searchPath + import->getBaseName("/") + ".strela", false});
    }

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
    Strela::g_homePath = std::string(getenv("HOMEDRIVE")) + getenv("HOMEPATH");
    #else
    Strela::g_homePath = getenv("HOME");
    #endif

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
        else if (!strcmp(argv[i], "--pretty")) pretty = true;
        else if (!strcmp(argv[i], "--timeout")) {
            g_timeout = std::strtol(argv[++i], nullptr, 10) * 1000;
        }
        else if (!strcmp(argv[i], "--search")) {
            g_searchPath = argv[++i];
        }
        else if (!strcmp(argv[i], "--write-bytecode")) {
            byteCodePath = argv[++i];
        }
        else {
            help();
            exit(1);
        }
    }
    
    if (fileName.empty()) {
        error("Expected file name as last cmd line argument.");
        help();
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

            std::map<std::string, ModDecl*> modules {
                { module->name, module },
            };

            std::vector<ModDecl*> processImports { module };
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