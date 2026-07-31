start procedure[ i64 : argument @@i8 ]
{
	$( argument.count ): exit_syscall[ 127 ], 
		1 : exit_syscall[ 0 ],
		2 : exit_syscall[ 1 ],
		3 : exit_syscall[ 2 ];
	!return[ 127 ];
}
