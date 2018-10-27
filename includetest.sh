#!/bin/bash
for f in $(find . -type f -name '*.h'); do
    echo $f
    printf "#include \"$f\"\nint main() { return 0; }\n" | g++ --std=c++11 -x c++ -o /dev/null -;
done