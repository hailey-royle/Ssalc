
# Ssalc - Ssal to llvmir compiler.

Ssalc is intended for bootstrapping a future self hosted compiler.  
It currently supports x86_64 Linux.

1. [Building Ssalc](#building-ssalc)
2. [Using Ssalc](#using-ssalc)
3. [Tests](#tests)
4. [Documentation](#documentation)


## Building Ssalc

`make build` - To compile Ssalc  
`clang source/ssal.c` - Also works


## Using Ssalc

`./ssalc file.sl` - To compile file.sl to file.ll

The file extension is ignored by the compiler, but by convention it should be .sl.


`clang -c -masm=intel source/_start.s` - To assemble _start.s

`_start.s` is included with every ssal program.
It moves the command line arguments into the format ssal code expects, and exits after the start procedure is returned from.
It only works with x86_64 Linux.


`clang file.ll _start.o -nostdlib -static` - To compile the final executable.

`-nostdlib` and `-static` are required, your *should* be able to use *some* other clang flags.

- `-o output_name` will work.
- optimization flags should work, albiet not optomizing as well as ir generated from c with clang.
- debug flags will not work, they need Ssalc support.
- linking: good luck!


## Tests

`make build test` - To run all tests

`make build test t=test_name` To run a specific test, with all ssalc and clang errors and output.

Tests are located in `./test`, each test is a subfolder in `./test`.
Every test has a `test.sh` script that runst the test, and a `test.sl` file which is the code to be tested.
On failure, ssalc and clang output is redirected to `./test/*test_name*/out.txt`.


## Documentation

The documentation is very crude and unorganized.

`ssalc/documentation/anatomy_of_a_hello_world.md`

Somewhat complete explanation of everything needed for a basic hello world program.


`ssalc/documentation/philosophy.md`

Basic outline of the priciples that defined ssal and what I value in a programming language.


`ssalc/documentation/refrence.md`

Crude and unfinished.
The refrence will be a knowledge base for all the ssal language and core.sl standard library.

