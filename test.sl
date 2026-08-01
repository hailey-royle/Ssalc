start procedure[ i32 : argc i32, argv ^^i8 ]{
		argument @@i8 = parse_command_line_argumnets[ argc, argv ];
		!loop_arguments[ 0 ];

	loop_arguments routine[ index i32 ];
		bytes i32 = write[ 1, argument@( index )^ ];
		?( index >= argument.count ): !return[ 0 ], !loop_arguments[ index + 1 ];
}

parse_command_line_argumnets procedure[ @@i8 : argc i32, argv ^^i8 ]{
		assert[ argc <> 0 ];
		assert[ argv <> 0 ];
		assert[ argv^ <> 0 ];
		argument @@i8 = { data = allocate[ stack, size_of[ @i8 ] * argc ]; count = argc; };
		!loop_argument[ 0, 0 ];

	loop_argument routine[ index i32, count i32 ];
		?( argv@( index )^@( count )^ == 0 ): !found_argument[], !loop_argument[ index, count + 1 ];

	found_argument routine[];
		next_argument ^@i8 = argument@( index );
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

