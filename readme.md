
## Ssalc

Ssal compiler written in c.

Outputs llvmir, intended for bootstrapping future self hosted compiler.

`make build` to compile

`make test` to run the tests


### Usage

`./ssalc file.sl`

Compiles file.sl to out.ll.
The .sl extension is optional, it can be removed or changed to anything.

`nasm -felf64 _start.s`

`_start.s` is included with every ssal program, it is required to set up the 'runtime enviorment'.
It moves the command line arguments into a format ssal code can use.
It is currently stored in ./test.

`clang out.ll _start.o -nostdlib -static`

Compiles the final executable.
`-nostdlib` and `-static` are required, your *should* be able to use *some* other clang flags.

- `-o output_name` will work.
- optimization flags should work, albiet they might not optomize as well as ir generated with c.
- debug flags should not work, needs ssalc support.
- linking, good luck!


### Tests

Tests are located in ./test.
To run all the tests, ./test.sh in the test directory.
Each test is a subfolder int ./test, with a test.sh script that runs the test.
On failure, ssalc compiler output is redirected to ./test/*test*/out.txt.


### Documentation

Currently a hot mess. I will get something together soon.

