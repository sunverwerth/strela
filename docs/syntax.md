# Strela Language Syntax

Every strela source file represents a module and must at its root contain a module declaration.

## Comments
```TS
/**
 * Block comments can span multiple lines
 */

// Line comments are terminated by a new line
```

## Module
```TS
module Foo.Bar.Test {
    // Import symbols into current scope
    import ...;

    // Define classes
    class Name {...}

    // Define interfaces
    interface Name {...}

    // Define enums
    enum name {...}

    // Define functions
    function name() {...}
}
```

## Imports
```TS
// import module A.B (if file A/B.strela exists) else symbol B from module A (if file A.strela exists)
import A.B;

// import all exported symbols from module A.B (if file A/B.strela exists)
import A.B.*;
```

## Classes
```TS
// Defines a class named Private that is private to the module
class Private {
}

// Defines class named Public that is exported from the module
export class Public {
}

class Foo {
    // Defines a constructor for the class
    function init() {}

    // Defines a method named `foo` for the class
    function foo() {}

    // Defines a method named `bar` for the class, returning an `int`
    function bar(): int { return 0; }

    // Defined a field named `field` of type `int`
    var field: int;
}
```

## Interfaces
```TS
[export] interface Bar {
    function member();
    var field: int;
}
```

## Enumeration
```TS
[export] enum Colors {
    Red, Green, Blue
}
```

## Generic Types
```TS
[export] class Generic<T> {
    var field: T;
    function member(arg: T) {}
}
```

## Functions
```TS
// Function `name` takes 2 `int` arguments and returns `bool`
function name(a: int, b: int): bool {
    return true;
}
```

## Entry Point
```TS
//No arguments
function main(): int {
    return 0;
}

// With cmd line arguments
function main(args: String[]): int { // cmd line arguments
    return 0;
}
```

## Variables
```TS
// Explicit type
var a: int;

// Explicit type with initializer
var a: int = 0;

// Implicit type from initializer
var a = 10;

// Allocate instance of Foo (calls Foo::init() if exists)
var obj = new Foo;

// Allocate instance of Foo with arguments (calls Foo::init(a,b,c))
var obj = new Foo(1, 2, 3);
```