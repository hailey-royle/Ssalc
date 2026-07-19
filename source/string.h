#ifndef STRINGH
#define STRINGH

struct string {
	char* data;
	i32 cap;
	i32 len;
};

bool string_from_file( struct string* source, char* filename ){
	assert( source != NULL, "Malformed args" );
	assert( source->cap == 0 && source->len == 0 && source->data == NULL, "String must be empty" );
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
	source->cap = file_length + 1;
	source->len = file_length;
	source->data[ source->len ] = '\0';
	return false;
}

bool string_to_file( struct string* source, char* filename ){
	assert( source != NULL, "Malformed args" );
	assert( source->cap > source->len || source->len <= 0, "Malformed internal source data" );
	assert( filename != NULL, "Malformed args" );
	FILE* file = fopen( filename, "w" );
	if( file == NULL ){
		return true;
	}
	i64 bytes_wrote = (i64) fwrite( source->data, 1, source->len, file );
	if( source->len != bytes_wrote ){
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
	source->cap = 0;
	source->len = 0;
}

void string_append( struct string* source, char* src, i32 count ){
	assert( source != NULL, "Malformed args" );
	assert( src != NULL, "Malformed args" );
	assert( source->cap > source->len || source->len <= 0, "Malformed internal source data" );
	if( count == 0 ) return;
	if( source->len + count >= source->cap ){
		i32 cap = ( source->cap + count ) * 2;
		char* tmp = realloc( source->data, cap );
		assert( tmp != NULL, "Alloc failed" );
		source->data = tmp;
		source->cap = cap;
	}
	memmove( &source->data[ source->len ], src, count );
	source->len += count;
	source->data[ source->len ] = '\0';
	return;
}

#endif
