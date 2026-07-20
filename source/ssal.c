#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#define i64 int64_t
#define i32 int32_t
#define i16 int16_t
#define i8 int8_t

#include "assert.h"
#include "string.h"
#include "tui.h"

enum raw_token_kind {
	error_token,
};

enum ast_node_kind {
	error_node,
	file_node,
};

struct raw_token {
	char* raw;
	i32 length;
	i32 line;
	i32 column;
	enum raw_token_kind kind;
};

struct ast_node {
	struct ast_node* sibling;
	struct ast_node* child;
	char* raw;
	i32 length;
	i32 line;
	i32 column;
	enum ast_node_kind kind;
};

struct source_file {
	struct ast_node* root;
	struct string source;
	i32 index;
};

enum compiler_error_level {
	halt_level,
	error_level,
	warning_level,
	note_level,
};

void compiler_error( struct source_file* file, struct ast_node* problem, enum compiler_error_level level, char* format, ... ){
	char* start = problem->raw - problem->column;
	i32 bytes_after_problem = 0;
	while( problem->raw[ bytes_after_problem ] != '\n' && problem->raw[ bytes_after_problem ] != '\r' && problem->raw[ bytes_after_problem ] != '\0' ){
		bytes_after_problem += 1;
	}
	if( level == halt_level ){
		printf( "%s%sError%s ", ansi_bold_start, ansi_foreground_magenta, ansi_foreground_default );
	} else if( level == error_level){
		printf( "%s%sError%s ", ansi_bold_start, ansi_foreground_red, ansi_foreground_default );
	} else if( level == warning_level ){
		printf( "%s%sWarning%s ", ansi_bold_start, ansi_foreground_yellow, ansi_foreground_default );
	} else if( level == note_level ){
		printf( "%s%sNote%s ", ansi_bold_start, ansi_foreground_green, ansi_foreground_default );
	}
	{
		va_list args;
		va_start( args, format );
		vprintf( format, args );
		va_end( args );
	}
	if( level == halt_level ){
		printf( "%s\n", ansi_bold_end );
		exit( 1 );
	}
	printf( "%s\n %*s | %d | %*s%s%*s%s%*s\n",
		ansi_bold_end, file->root->length, file->root->raw, problem->line,
		problem->column, start, ansi_underline_start,
	        problem->length, start + problem->column, ansi_underline_end,
		bytes_after_problem, start + problem->column + problem->length
	);
	exit( 1 );
}

void parse_file( struct ast_node* root_node ){
	assert( root_node != NULL, "Malformed argument." );
	assert( root_node->child == NULL, "Malformed argument data." );
	assert( root_node->raw != NULL, "Malformed argument data." );
	assert( root_node->length > 0, "Malformed argument data." );
	assert( root_node->kind == file_node, "Malformed argument data." );
	struct source_file file = {
		.root = root_node,
		.source = { 0 },
		.index = 0,
	};
	bool error = string_from_file( &file.source, file.root->raw );
	if( error ){
		compiler_error( &file, file.root, halt_level, "Unable to open file \"%s\"", root_node->raw );
	}
	struct raw_token token = next_token( &file );
	while( 1 ){
		break;
	}
}

i32 main( i32 argc, char* argv[] ){
	if( argc != 2 ){
		fprintf( stderr, "Usage: ssalc file_name.sl\n" );
		exit( 1 );
	}
	struct ast_node root_node = {
		.sibling = NULL,
		.child = NULL,
		.raw = argv[ 1 ],
		.length = strlen( argv[ 1 ]),
		.line = -1,
		.column = -1,
		.kind = file_node,
	};
	parse_file( &root_node );
//	validate_globals( root_node );
//	output( root_node );
	return 0;
}

	
