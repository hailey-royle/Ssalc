start procedure[ i64 : argument @@i8 ]
{
	?( argument.count == 1 ): exit_syscall[ 0 ], exit_syscall[ 1 ];
	!return[ 127 ];
}
