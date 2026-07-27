start procedure[ i64 : argument @@i8 ]{
	bytes i64 = write_syscall[ 1, argument@ 0 ];
	!return[ 0 ];
}
