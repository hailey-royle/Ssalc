#ifndef ARRAYH
#define ARRAYH

#include "assert.h"

struct array {
	void* data;
	uint32_t count;
	uint32_t allocated;
};

void* array_new( struct array* array, size_t sizeof_element ){
        assert( array != NULL, "Malformed argument." );
        assert( array->count <= array->allocated, "Malformed argument." );
        if( array->count == array->allocated ){
                int new_allocated = ( array->allocated == 0 ) ? 256 : array->allocated * 2;
                void* tmp = realloc( array->data, sizeof_element * new_allocated );
                assert( tmp != NULL, "alloc failed." );
                memset( tmp + sizeof_element * array->allocated, 0, sizeof_element * ( new_allocated - array->allocated ));
                array->allocated = new_allocated;
                array->data = tmp;
        }
        array->count += 1;
        return array->data + sizeof_element * ( array->count - 1 );
}

#endif
