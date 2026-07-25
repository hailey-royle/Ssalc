#!/bin/bash

clean_dir () {
	rm test test.ll test.log test.txt >/dev/null 2>&1
}

output () {
	if [ "$1" == "passed" ]; then
		printf "%-*s [\e[32m %s \e[0m]\n" 24 "${PWD##*/}" "$1"
		clean_dir
	else
		printf "%-*s [\e[31m %s \e[0m]\n" 24 "${PWD##*/}" "$1"
	fi
	exit
}

compile_sl () {
	../../ssalc test.sl >test.log 2>&1
	if [ $? != 0 ]; then
		output "ssalc failed"
	fi
}

compile_ll () {
	clang test.ll ../../_start.o -nostdlib -static -o test >test.log 2>&1
	if [ $? != 0 ]; then
		output "clang failed"
	fi
}

pre_test () {
	clean_dir
	compile_sl
	compile_ll
}
