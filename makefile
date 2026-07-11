SHELL := /bin/bash
# t := test_name

build:
	time gcc source/ssal.c -o ssalc -std=c23 -Wall -Wextra -Wpedantic -fanalyzer -O3
debug:
	time gcc source/ssal.c -o ssalc -std=c23 -Wall -Wextra -Wpedantic -fanalyzer -ggdb -fsanitize=address,leak,undefined
test:
ifdef t
	./ssalc test/$(t)/test.sl
	clang out.ll test/_start.o -nostdlib -static -o $(t)
else
	$(SHELL) -c "cd test && ./test.sh"
endif
