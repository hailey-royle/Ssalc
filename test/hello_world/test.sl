start procedure[ argument @@i8 : i64 ]
{
	bytes_written i64 = write_syscall[ 1, "Hello World!", 12 ];
	!return[ 0 ];
}
