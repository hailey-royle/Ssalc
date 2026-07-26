#ifndef ASSERTH
#define ASSERTH

#define assert( expr, ... ){ \
	if( !( expr )){ \
		fprintf( stderr, "%s:%d: %s: assertion \"%s\" failed. ", __FILE__, __LINE__, __func__, #expr ); \
		fprintf( stderr, __VA_ARGS__ ); \
		fprintf( stderr, "\r\n" ); \
		fflush( stderr ); \
		exit( 1 ); \
	} \
}

#define unreachable { \
	fprintf( stderr, "%s:%d: %s: Unrechable reached.\r\n", __FILE__, __LINE__, __func__ ); \
	exit( 1 ); \
}

#endif
