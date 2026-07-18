#ifndef ASSERTH
#define ASSERTH

#define assert( expr, ... ){ \
	if( !( expr )){ \
		fprintf( stderr, "%s:%d: %s: assertion \"%s\" failed. ", __FILE__, __LINE__, __func__, #expr ); \
		fprintf( stderr, __VA_ARGS__ ); \
		fprintf( stderr, "\r\n" ); \
		fflush( stderr ); \
		exit( 1 );\
	} \
}

#endif
