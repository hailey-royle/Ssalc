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
	identifier_token,
	procedure_token,
	routine_token,
	structure_token,
	union_token,
	enumeration_token,
	literal_number_token,
	literal_string_token,
	literal_code_token,
	argument_open_token,
	argument_close_token,
	expression_open_token,
	expression_close_token,
	scope_open_token,
	scope_close_token,
	list_separator_token,
	statement_end_token,
	result_token,
	jump_token,
	assignment_token,
	interpreter_token,
	pointer_token,
	array_token,
	conditional_token,
	selection_token,
	member_token,
	addition_token,
	subtraction_token,
	multiplication_token,
	division_token,
	modulo_token,
	shift_right_token,
	shift_left_token,
	bit_not_token,
	bit_and_token,
	bit_or_token,
	bit_xor_token,
	logical_not_token,
	logical_and_token,
	logical_or_token,
	equal_token,
	less_token,
	greater_token,
	less_equal_token,
	greater_equal_token,
	less_greater_token,
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
	i32 line;
	i32 column;
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
	if(( level == halt_level ) || ( level == error_level )){
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

bool char_is_space( char c ){
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

bool char_is_identifier_start( char c ){
	return ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || c == '_';
}

bool char_is_identifier( char c ){
	return ( c >= '0' && c <= '9' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || c == '_';
}

bool char_is_integer_start( char c ){
	return c >= '0' && c <= '9' ;
}

bool char_is_integer( char c ){
	return ( c >= '0' && c <= '9' ) || c == '_';
}

bool char_array_equal( char* a, char* b, i32 n ){
	for( i32 i = 0; i < n; i++ ){
		if( a[ i ] == 0 || b[ i ] == 0 || a[ i ] != b[ i ] ){
			return false;
		}
	}
	return true;
}

struct raw_token next_token( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->root != NULL, "Malformed argument data." );
	assert( file->source.data != NULL, "Malformed argument data." );
	assert( file->index >= 0, "Malformed argument data." );
	assert( file->source.length > file->index, "Malformed argument data." );
	struct raw_token token = { 0 };
	while( char_is_space( file->source.data[ file->index ])){
		if( '\n' == file->source.data[ file->index ]){
			file->column = 0;
			file->line += 1;
		} else {
			file->column += 1;
		}
		file->index += 1;
	}
	if( file->index >= file->source.length ){
		return token;
	}
	if(( '\\' == file->source.data[ file->index ]) && ( '{' != file->source.data[ file->index + 1 ])){
		bool in_string = false;
		do{
			if( '\n' == file->source.data[ file->index ]){
				file->column = 1;
				file->line += 1;
			} else {
				file->column += 1;
			}
			file->index += 1;
			if(( '"' == file->source.data[ file->index ]) && ( '\\' != file->source.data[ file->index - 1 ])){
				in_string = ( in_string == true ) ? false : true;
			}
		} while(( in_string == false ) && ( '}' == file->source.data[ file->index ]) && ( '\\' != file->source.data[ file->index + 1 ]));
		file->column += 2;
		file->index += 2;
		if( '"' == file->source.data[ file->index ]){
			struct ast_node fake_node = {
				.sibling = NULL,
				.child = NULL,
				.raw = token.raw,
				.length = token.length,
				.line = token.line,
				.column = token.column,
				.kind = error_node
			};
	                compiler_error( file, &fake_node, halt_level, "String literal must not directly follow comment. (Hint: add ' ' after comment)" );
		}
	}
	token.raw = &file->source.data[ file->index ];
	token.column = file->column;
	token.line = file->line;
	if( char_is_identifier_start( file->source.data[ file->index ])){
		do{
			file->column += 1;
			file->index += 1;
			token.length += 1;
		} while( char_is_identifier( file->source.data[ file->index ]));
		if( char_array_equal( token.raw, "procedure", 9 )){
			token.kind = procedure_token;
		} else if( char_array_equal( token.raw, "routine", 7 )){
			token.kind = routine_token;
		} else if( char_array_equal( token.raw, "structure", 9 )){
			token.kind = structure_token;
		} else if( char_array_equal( token.raw, "union", 5 )){
			token.kind = union_token;
		} else if( char_array_equal( token.raw, "enumeration", 11 )){
			token.kind = enumeration_token;
		} else {
			token.kind = identifier_token;
		}
	} else if( char_is_integer_start( file->source.data[ file->index ])){
		do{
			file->column += 1;
			file->index += 1;
			token.length += 1;
		} while( char_is_integer( file->source.data[ file->index ]));
		token.kind = literal_number_token;
	} else if( '"' == file->source.data[ file->index ]){
		do{
			if( '\n' == file->source.data[ file->index ]){
				file->column = 1;
				file->line += 1;
			} else {
				file->column += 1;
			}
			file->index += 1;
			token.length += 1;
		} while( '"' != file->source.data[ file->index ]);
                file->column += 1;
                file->index += 1;
                token.length += 1;
                token.kind = literal_string_token;
	} else if( '\'' == file->source.data[ file->index ]){
		bool in_string = false;
		do{
			if( '\n' == file->source.data[ file->index ]){
				file->column = 1;
				file->line += 1;
			} else {
				file->column += 1;
			}
			file->index += 1;
			token.length += 1;
			if(( '"' == file->source.data[ file->index ]) && ( '\\' != file->source.data[ file->index - 1 ])){
				in_string = ( in_string == true ) ? false : true;
			}
		} while(( in_string == false ) && ( '\'' != file->source.data[ file->index ]));
                file->column += 1;
                file->index += 1;
                token.length += 1;
                token.kind = literal_code_token;
	} else if( '[' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = argument_open_token;
	} else if( ']' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = argument_close_token;
	} else if( '(' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = expression_open_token;
	} else if( ')' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = expression_open_token;
	} else if( '{' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = scope_open_token;
	} else if( '}' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = scope_close_token;
	} else if( ',' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = list_separator_token;
	} else if( ';' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = statement_end_token;
	} else if( ':' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = result_token;
	} else if( '!' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = jump_token;
	} else if( '=' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		if( '=' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = equal_token;
		} else {
			token.kind = assignment_token;
		}
	} else if( '#' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = interpreter_token;
	} else if( '^' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = pointer_token;
	} else if( '@' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = array_token;
	} else if( '?' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = conditional_token;
	} else if( '$' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = selection_token;
	} else if( '.' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = member_token;
	} else if( '+' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = addition_token;
	} else if( '-' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = subtraction_token;
	} else if( '*' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = multiplication_token;
	} else if( '/' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = division_token;
	} else if( '*' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = modulo_token;
	} else if( '~' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		if( '~' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = logical_not_token;
		} else {
			token.kind = bit_not_token;
		}
	} else if( '&' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		if( '&' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = logical_and_token;
		} else {
			token.kind = bit_and_token;
		}
	} else if( '|' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		if( '|' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = logical_or_token;
		} else {
			token.kind = bit_or_token;
		}
	} else if( '`' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		token.kind = bit_xor_token;
	} else if( '<' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		if( '<' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = shift_left_token;
		} else if( '=' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = less_equal_token;
		} else {
			token.kind = less_token;
		}
	} else if( '>' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		token.length += 1;
		if( '>' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = shift_right_token;
		} else if( '=' == file->source.data[ file->index ]){
			file->column += 1;
			file->index += 1;
			token.length += 1;
			token.kind = greater_equal_token;
		} else {
			token.kind = greater_token;
		}
        } else {
		struct ast_node fake_node = {
			.sibling = NULL,
			.child = NULL,
			.raw = token.raw,
			.length = token.length,
			.line = token.line,
			.column = token.column,
			.kind = error_node
		};
                compiler_error( file, &fake_node, halt_level, "Unable to parse syntax." );
        }
	return token;
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
		.line = 1,
		.column = 0,
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

