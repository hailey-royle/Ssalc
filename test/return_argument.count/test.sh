#!/bin/bash

output () {
	if [ "$1" == "passed" ]; then
		printf "%-*s [\e[32m %s \e[0m]\n" 24 "${PWD##*/}" "$1"
		rm out.txt test.ll test &>/dev/null
	else
		printf "%-*s [\e[31m %s \e[0m]\n" 24 "${PWD##*/}" "$1"
	fi
	exit
}

# clean directory
rm out.txt test.ll test &>/dev/null

# compile test.sl to test.ll, redirect any errors to out.txt.
../../ssalc test.sl &>out.txt
if [ $? != 0 ]; then
	output "ssalc failed"
fi

# compile test.ll to test with normal flags, redirect any errors to out.txt.
clang test.ll ../../_start.o -nostdlib -static -o test &>out.txt
if [ $? != 0 ]; then
	output "clang failed"
fi

# run the test
./test two 3 foura08d8ha0sdufh
if [ $? == 4 ]; then
	output "passed"
else
	output "executable failed"
fi

