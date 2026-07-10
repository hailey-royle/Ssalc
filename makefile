SHELL := /bin/bash

build:
	time gcc source/ssal.c -o ssalc -std=c23 -Wall -Wextra -Wpedantic -fanalyzer -O3
debug:
	time gcc source/ssal.c -o ssalc -std=c23 -Wall -Wextra -Wpedantic -fanalyzer -ggdb -fsanitize=address,leak,undefined
test:
	$(SHELL) -c "cd test && ./test.sh && cd .."
