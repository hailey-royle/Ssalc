start procedure[ argument @@u8 : u8 ]
{
	write_syscall[ 1, "Hello World!", 12 ];
	!return[ 0 ];
}
