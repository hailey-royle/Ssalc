#!/bin/bash

# clean directory
rm out.txt test.ll test &>/dev/null

# compile test.sl to test.ll, redirect any errors to out.txt.
../../ssalc test.sl &>out.txt
if [ $? != 0 ]; then
	echo -e "${PWD##*/} \t[\e[31m ssalc failed \e[0m]"
	exit
fi

# compile test.ll to test with normal flags, redirect any errors to out.txt.
clang test.ll ../_start.o -nostdlib -static -o test &>out.txt
if [ $? !=  0 ]; then
	echo -e "${PWD##*/} \t[\e[31m clang failed \e[0m]"
	exit
fi

# run the test
./test two 3 four7989yg9gyvg9gy89gy9giyu
if [[ $? == 4 ]]; then
	echo -e "${PWD##*/} \t[\e[32m passed \e[0m]"
else
	echo -e "${PWD##*/} \t[\e[31m executable failed \e[0m]"
fi

# clean directory
rm out.txt test.ll test &>/dev/null
