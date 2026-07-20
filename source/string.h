#ifndef STRINGH
#define STRINGH

struct string {
	char* data;
	i32 allocated;
	i32 length;
};

bool string_from_file( struct string* source, char* filename ){
	assert( source != NULL, "Malformed args" );
	assert( source->allocated == 0 && source->length == 0 && source->data == NULL, "String must be empty" );
	assert( filename != NULL, "Malformed args" );
	FILE* file = fopen( filename, "r" );
	if( file == NULL ){
		return true;
	};
	assert( file != NULL, "fopen failed" );
	i32 error = fseek( file, 0, SEEK_END );
	if( error != 0 ){
		return true;
	};
	i32 file_length = ftell( file );
	rewind( file );
	source->data = malloc( file_length + 1 );
	if( source->data == NULL ){
		return true;
	};
	i64 bytes_read = (i64) fread( source->data, 1, file_length, file );
	if( file_length != bytes_read ){
		return true;
	};
	fclose( file );
	source->allocated = file_length + 1;
	source->length = file_length;
	source->data[ source->length ] = '\0';
	return false;
}

bool string_to_file( struct string* source, char* filename ){
	assert( source != NULL, "Malformed args" );
	assert( source->allocated > source->length || source->length <= 0, "Malformed internal source data" );
	assert( filename != NULL, "Malformed args" );
	FILE* file = fopen( filename, "w" );
	if( file == NULL ){
		return true;
	}
	i64 bytes_wrote = (i64) fwrite( source->data, 1, source->length, file );
	if( source->length != bytes_wrote ){
		return true;
	}
	fclose( file );
	return false;
}

void string_free( struct string* source ){
	assert( source != NULL, "Malformed args" );
	if( source->data != NULL ){
		free( source->data );
		source->data = NULL;
	}
	source->allocated = 0;
	source->length = 0;
}

void string_append( struct string* source, char* src, i32 count ){
	assert( source != NULL, "Malformed args" );
	assert( src != NULL, "Malformed args" );
	assert( source->allocated > source->length || source->length <= 0, "Malformed internal source data" );
	if( count == 0 ) return;
	if( source->length + count >= source->allocated ){
		i32 allocated = ( source->allocated + count ) * 2;
		char* tmp = realloc( source->data, allocated );
		assert( tmp != NULL, "Alloc failed" );
		source->data = tmp;
		source->allocated = allocated;
	}
	memmove( &source->data[ source->length ], src, count );
	source->length += count;
	source->data[ source->length ] = '\0';
	return;
}

#endif
