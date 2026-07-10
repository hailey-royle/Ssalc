../../ssalc test.sl >out.txt &&
clang out.ll ../_start.o -nostdlib -static -o test &&
rm out.ll &&
./test
if [ $? -eq 0 ]; then
	echo -e "${PWD##*/} [ \e[32mpassed\e[0m ]"
else
	echo -e "${PWD##*/} [ \e[31mfailed\e[0m ]"
fi
