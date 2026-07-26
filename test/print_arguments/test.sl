start procedure[ i64 : argument @@i8 ]{
		!loop[ 0 ];
	loop routine[ i i64 ];
		bytes i64 = write[ 1, argument@ i ];
		? i < argument.count : !loop[ i + 1 ], !return[ 0 ];
}
