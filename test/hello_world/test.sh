# compile the test program to test.sl, redirect any errors to out.txt and report any error.
../../ssalc test.sl >out.txt
if [ $? != 0 ]; then
	echo -e "${PWD##*/} [ \e[31mssalc failed\e[0m ]"
	exit
fi

# compile out.ll to test with normal flags, redirect any errors to out.txt and report any error
clang out.ll ../_start.o -nostdlib -static -o test >>out.txt
if [ $? !=  0 ]; then
	echo -e "${PWD##*/} [ \e[31mclang failed\e[0m ]"
	exit
fi

# run the test
./test >>test.txt
if [[ "$(< test.txt)" == "Hello World!" ]]; then
	echo -e "${PWD##*/} [ \e[32mpassed\e[0m ]"
else
	echo -e "${PWD##*/} [ \e[31mprogram failed\e[0m ]"
fi

# clean directory
rm out.txt out.ll test >/dev/null 2>/dev/null
