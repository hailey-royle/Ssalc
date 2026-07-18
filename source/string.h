#ifndef STRINGH
#define STRINGH

struct string {
	char* data;
	size_t cap;
	size_t len;
};

bool string_from_file( struct string* source, char* filename ){
	assert( source != NULL, "Malformed args" );
	assert( source->cap == 0 && source->len == 0 && source->data == NULL, "String must be empty" );
	assert( filename != NULL, "Malformed args" );
	struct stat stat_buffer = { 0 };
	int stat_err = stat( filename, &stat_buffer );
	if( stat_err == -1 ){
		char* tmp = malloc( 1 );
		assert( tmp != NULL, "Alloc failed." );
		source->data = tmp;
		source->cap = 1;
		source->len = 0;
		source->data[ source->len ] = '\0';
	} else {
		if( !S_ISREG( stat_buffer.st_mode )){
			return true;
		}
		FILE* file = fopen( filename, "r" );
		assert( file != NULL, "fopen failed" );
		char* tmp = malloc( stat_buffer.st_size + 1 );
		assert( tmp != NULL, "Alloc failed." );
		source->data = tmp;
		size_t fread_res = fread( source->data, 1, stat_buffer.st_size, file );
		assert( (size_t) stat_buffer.st_size == fread_res, "fread failed" );
		fclose( file );
		source->cap = stat_buffer.st_size + 1;
		source->len = stat_buffer.st_size ;
		source->data[ source->len ] = '\0';
	}
	return false;
}

bool string_to_file( struct string* source, char* filename ){
	assert( source != NULL, "Malformed args" );
	assert( source->cap > source->len || source->len <= 0, "Malformed internal source data" );
	assert( filename != NULL, "Malformed args" );
	struct stat stat_buffer = { 0 };
	int stat_err = stat( filename, &stat_buffer );
	if( stat_err != -1 && !S_ISREG( stat_buffer.st_mode )){
		return true;
	}
	FILE* file = fopen( filename, "w" );
	assert( file != NULL, "fopen failed" );
	size_t fwrite_res = fwrite( source->data, 1, source->len, file );
	assert( source->len == fwrite_res, "fwrite failed" );
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

void string_append( struct string* source, char* src, size_t count ){
	assert( source != NULL, "Malformed args" );
	assert( src != NULL, "Malformed args" );
	assert( source->cap > source->len || source->len <= 0, "Malformed internal source data" );
	if( count == 0 ) return;
	if( source->len + count >= source->cap ){
		size_t cap = ( source->cap + count ) * 2;
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
