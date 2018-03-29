# Strela Language Syntax

Every strela source file represents a module and must at its root contain a module declaration.

## Comments
    /**
     * Block comments can span multiple lines
     */

    // Line comments are terminated by a new line

## Module
    module Test {
        // imports
        // classes
        // interfaces
        // enums
        // functions
    }

## Imports
    // import module A.B or symbol B from module A whichever matches first
    import A.B;

    // import all exported symbols from module A.B
    import A.B.*;


## Classes
    // Define a class named Private that is private to the module
    class Private {
    }

    // Define a class named Public that is exported from the module
    export class Public {
    }

    class Foo {
        // define a field named `field` of type `int`
        var field: int;
    }

## Interfaces
    [export] interface Bar {
        function member();
    }

## Enumeration
    [export] enum Colors {
        Red, Green, Blue
    }

## Generic Types
    [export] class Generic<T> {
        var field: T;
        function member(arg: T) {}
    }

## Functions
Function `name` takes 2 `int` arguments and returns `bool`

    function name(a: int, b: int): bool {
        return true;
    }


## Entry Point
No arguments

    function main(): int {
        return 0;
    }

cmd line arguments

    function main(args: String[]): int { // cmd line arguments
        return 0;
    }

## Variables

    var a: int;
    var a: int = 0;
    var a = 10;

    var obj = new Foo;
    var obj = new Foo(1, 2, 3);

