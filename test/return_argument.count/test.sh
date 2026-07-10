# compile the test program to test.sl, redirect any errors or warnings to out.txt and stop if error.
../../ssalc test.sl >out.txt &&

# compile out.ll to test, normal flags, stop if error
clang out.ll ../_start.o -nostdlib -static -o test >>out.txt &&

# run the test
./test two 3 foiuas0d8h0a9

# evaulate if the test passed and print the result
if [ $? -eq 4 ]; then
	echo -e "${PWD##*/} [ \e[32mpassed\e[0m ]"
else
	echo -e "${PWD##*/} [ \e[31mfailed\e[0m ]"
fi

# clean directory
rm out.txt out.ll test >/dev/null 2>/dev/null

