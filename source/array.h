#ifndef ARRAYH
#define ARRAYH

/*====== Using array.h ========

**Replace all instanced of type below with the type of the array.**

  // Assumes this i32 is defined to int32_t, different types can be used if desired.
#define i32 int32_t

  // Recomended to put this struct elsewhere in the program, else it must be above its corresponding functions.
struct type_array {
	type* data;
	i32 count;
	i32 allocated;
};

  // Must be after all array struct definitions.
#include "array.h"

  // Copy this function below.
struct ast_node* ast_node_array_new( struct ast_node_array* array ){
        assert( array != NULL, "Malformed argument." );
        assert( array->count >= 0, "Malformed data." );
        assert( array->allocated >= 0, "Malformed data." );
        assert( array->count <= array->allocated, "Malformed data." );
        if( array->count == array->allocated ){
                i32 new_allocated = ( array->allocated == 0 ) ? 256 : array->allocated * 2;
                array->data = realloc( array->data, sizeof( array->data[ 0 ]) * new_allocated );
                assert( array->data != NULL, "alloc failed." );
                memset( &array->data[ array->allocated ], 0, sizeof( array->data[ 0 ] ) * ( new_allocated - array->allocated ));
                array->allocated = new_allocated;
        }
        array->count += 1;
        return &array->data[ array->count - 1 ];
}

======== Using array.h ======*/

struct ast_node* ast_node_array_new( struct ast_node_array* array ){
        assert( array != NULL, "Malformed argument." );
        assert( array->count >= 0, "Malformed data." );
        assert( array->allocated >= 0, "Malformed data." );
        assert( array->count <= array->allocated, "Malformed data." );
        if( array->count == array->allocated ){
                i32 new_allocated = ( array->allocated == 0 ) ? 256 : array->allocated * 2;
                array->data = realloc( array->data, sizeof( array->data[ 0 ]) * new_allocated );
                assert( array->data != NULL, "alloc failed." );
                memset( &array->data[ array->allocated ], 0, sizeof( array->data[ 0 ] ) * ( new_allocated - array->allocated ));
                array->allocated = new_allocated;
        }
        array->count += 1;
        return &array->data[ array->count - 1 ];
}

#endif
