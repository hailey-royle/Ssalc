#include[ "core.sl" ];

start procedure[ i32 : argc i32, argv ^^i8 ]{
		argument array[ array[ i8 ]] = parse_command_line_argumnets[ argc, argv ];
		!loop_arguments[ 0 ];

	loop_arguments routine[ index i32 ];
		bytes i32 = write[ 1, argument.data@( index )^ ];
		?( index >= argument.count ): !return[ 0 ], !loop_arguments[ index + 1 ];
}
