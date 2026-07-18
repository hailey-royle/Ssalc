SHELL := /bin/bash
# t := test_name

build:
	clang -c -masm=intel source/_start.s
	time clang source/ssal.c -o ssalc -std=c23 -Wall -Wextra -pedantic -Og -ggdb
asan:
	clang -c -masm=intel source/_start.s
	time clang source/ssal.c -o ssalc -std=c23 -Wall -Wextra -pedantic -Og -ggdb -fsanitize=address,leak,undefined
test:
ifdef t
	./ssalc test/$(t)/test.sl
	clang test/$(t)/test.ll _start.o -nostdlib -static -o $(t)
else
	time $(SHELL) -c "cd test && ./test.sh"
endif
