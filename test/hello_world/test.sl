start procedure[ i64 : argument @@i8 ]
{
	bytes_written i64 = write_syscall[ 1, "Hello World!", 12 ];
	!return[ 0 ];
}
