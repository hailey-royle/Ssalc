nasm -felf64 _start.s;

for D in */; do
	( cd ${D}; ./test.sh; )
done

