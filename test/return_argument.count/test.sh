../../ssalc test.sl >out.txt &&
clang out.ll ../_start.o -nostdlib -static -o test &&
rm out.ll &&
./test two three four
if [ $? -eq 4 ]; then
	echo -e "${PWD##*/} [ \e[32mpassed\e[0m ]"
else
	echo -e "${PWD##*/} [ \e[31mfailed\e[0m ]"
fi

