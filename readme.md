
## ssalc

Ssal compiler written in c.

Outputs llvmir, intended for bootstrapping future self hosted compiler.

`make build` to compile

`make test` to run the tests


### tests

Tests are located in ./test.
To run all the tests, `$ ./test.sh` in the test directory.
Each test is a subfolder int `./test`, with a `test.sh` script that runs the test.
`_start.s` is linked with every ssal program, it is required to set up the runtime enviorment ( moves the command line arguments into a format ssal code can use ).
On failure, ssalc compiler output is output to ./test/*test*/out.txt.

