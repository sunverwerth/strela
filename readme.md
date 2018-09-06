# Strela programming language
Strela is a static and strongly typed object oriented programming language with heavy focus on composition and language assisted delegation.

***Development status is pre-pre-pre-alpha! Here be dragons!***

[![Build Status](https://travis-ci.org/MadHed/strela.svg?branch=master)](https://travis-ci.org/MadHed/strela)

## Setup  
    git clone git@github.com:MadHed/strela.git  
    cd strela  
    make test && make install-home

The included MSVC project file has been tested with Visual Studio Community 2017.

## Basic usage
    strela [options] input-file

where `input-file` is either a .strela module or a module compiled to strela bytecode.
    
## Command line options
    --dump             dumps decompiled bytecode to stdout and exits.
    --pretty           pretty-prints the parsed code to stdout and exits.
    --timeout <sec>    kills the running program after <sec> seconds.
    --search <path>    sets additional search path <path> for imports.
    --write-bytecode <file>    writes compiled bytecode to <file> and exits.

## Examples

### Hello.strela
    module Hello {
        import Std.IO.println;

        function main(): int {
            println("Hello, world!");
            return 0;
        }
    }

### Fib.strela
    module Fib {
        import Std.IO.println;

        function fib(n: int): int {
            if (n < 2) {
                return n;
            }
            else {
                return fib(n - 1) + fib(n - 2);
            }
        }

        function main(): int {
            println(fib(40));
            return 0;
        }
    }
