array structure[ type ] {
	data ^type;
	count i32;
	allocated i32;
}

\{
array_new procedure[ ^array_type : arr ^array[ array_type type ]]{
}
	arr ^array[ i64 ] = allocate_array[ os, i64, 256 ];
	new ^i64 = array_new[ arr ];
}\

parse_command_line_argumnets procedure[ array[ array[ i8 ]] : argc i32, argv ^^i8 ]{
		assert[ argc <> 0 ];
		assert[ argv <> 0 ];
		assert[ argv^ <> 0 ];
		argument array[ array[ i8 ]] = { data = allocate[ stack, size_of[ array[ i8 ]] * argc ]; count = argc; };
		!loop_argument[ 0, 0 ];

	loop_argument routine[ index i32, count i32 ];
		?( argv@( index )^@( count )^ == 0 ): !found_argument[], !loop_argument[ index, count + 1 ];

	found_argument routine[];
		next_argument ^array[ i8 ] = argument.data@( index );
		next_argument^ = { data = argv@( index )^; count = count; };
		?( argv@( index )^ == 0 ): !return[ argument ], !find_argument[ index + 1, 0 ];
}

assert procedure[ : expression i64 ]{
		?( expression ): !return[], !failed[];

	failed routine[];
		bytes i64 = write[ 2, "Assert Failed.\n" ];
		exit[ 1 ];
		!return[];
}

\{
assert procedure[ : expression code ]{
		?( expression ): !return[], !failed[];

	failed routine[];
		bytes i64 = write_format[ 2, "%:%:% Assert '%' Failed.\n", { _calle_file_, _calle_procedure_, _calle_line_, expression }];
		exit[ 1 ];
		!return[];
}

write_format procedure[ file i32, format @i8, list @any ]{ ... }
}\

