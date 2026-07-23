SHELL := /bin/bash
# t := test_name

_start:
	clang -c -masm=intel source/_start.s

build: _start
	time clang source/ssal.c -o ssalc -std=c23 -Wall -Wextra -pedantic -Og -ggdb -DDEBUG

release: _start
	time clang source/ssal.c -o ssalc -std=c23 -Wall -Wextra -pedantic -O3

asan: _start
	time clang source/ssal.c -o ssalc -std=c23 -Wall -Wextra -pedantic -Og -ggdb -fsanitize=address -DDEBUG

test:
ifdef t
	./ssalc test/$(t)/test.sl
	clang test/$(t)/test.ll _start.o -nostdlib -static -o $(t)
else
	time $(SHELL) -c "cd test && ./test.sh"
endif
