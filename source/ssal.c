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
	procedure_node,
	routine_node,
	structure_node,
	union_node,
	enumeration_node,
	argument_node,
	register_node,
	type_node,
	jump_node,
	call_node,
	literal_number_node,
	member_node,
	addition_node,
	// not exaustive
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

struct ast_node_array {
	struct ast_node* data;
	i32 count;
	i32 allocated;
};

struct source_file {
	struct ast_node_array node_raw;
	struct string source;
	struct ast_node* root;
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

#include "array.h"

struct ast_node root_node = { 0 };

void print_ast_node( struct ast_node* node, i32 depth ){
        assert( node != NULL, "Malformed argument." );
	char* kind = NULL;
	if ( node->kind == error_node ){
		kind = "error";
	} else if ( node->kind == file_node ){
		kind = "file";
	} else if ( node->kind == procedure_node ){
		kind = "procedure";
	} else if ( node->kind == routine_node ){
		kind = "routine";
	} else if ( node->kind == structure_node ){
		kind = "structure";
	} else if ( node->kind == union_node ){
		kind = "union";
	} else if ( node->kind == enumeration_node ){
		kind = "enumeration";
	} else if ( node->kind == argument_node ){
		kind = "argument";
	} else if ( node->kind == register_node ){
		kind = "register";
	} else if ( node->kind == type_node ){
		kind = "type";
	} else if ( node->kind == jump_node ){
		kind = "jump";
	} else if ( node->kind == call_node ){
		kind = "call";
	} else if ( node->kind == literal_number_node ){
		kind = "literal_number";
	} else if ( node->kind == member_node ){
		kind = "member";
	} else if ( node->kind == addition_node ){
		kind = "addition";
	} else {
		assert( false, "Unknown node kind" );
	}
        printf( "%*c%s | %.*s\n", depth, ' ', kind, node->length, node->raw );
        if( node->child != NULL ){
                print_ast_node( node->child, depth + 1 );
        }
        if( node->sibling != NULL ){
                print_ast_node( node->sibling, depth );
        }
}

void print_ast(){
	printf( "=== Ast Start ===\n" );
        print_ast_node( &root_node, 1 );
	printf( "==== Ast End ====\n\n" );
}

void compiler_error( struct source_file* file, struct ast_node* problem, enum compiler_error_level level, char* format, ... ){
        assert( file != NULL, "Malformed argument." );
        assert( problem != NULL, "Malformed argument." );
        assert( problem->raw != NULL, "Malformed argument." );
	assert( problem->length > 0, "Malformed argument data." );
        assert( format != NULL, "Malformed argument." );
	char* start = problem->raw - problem->column;
	i32 bytes_after_problem = 0;
	while(( problem->raw[ bytes_after_problem + problem->length ] != '\n' ) &&
	      ( problem->raw[ bytes_after_problem + problem->length ] != '\r' ) &&
	      ( problem->raw[ bytes_after_problem + problem->length ] != '\0' )){
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
	printf( "%s\n  %.*s | %d | %.*s%s%.*s%s%.*s\n\n",
		ansi_bold_end, file->root->length, file->root->raw, problem->line,
		problem->column, start, ansi_underline_start ansi_foreground_red,
	        problem->length, start + problem->column, ansi_reset_graphics,
		bytes_after_problem, start + problem->column + problem->length
	);
#ifdef DEBUG
	print_ast();
#endif
	exit( 1 );
}

void compiler_error_token( struct source_file* file, struct raw_token* problem, enum compiler_error_level level, char* format, ... ){
        assert( file != NULL, "Malformed argument." );
        assert( problem != NULL, "Malformed argument." );
        assert( problem->raw != NULL, "Malformed argument." );
	assert( problem->length > 0, "Malformed argument data." );
        assert( format != NULL, "Malformed argument." );
	struct ast_node fake = {
		.raw = problem->raw,
		.length = problem->length,
		.line = problem->line,
		.column = problem->column,
		.kind = error_node
	};
	va_list args;
	va_start( args, format );
	compiler_error( file, &fake, level, format, args );
	va_end( args );
}

bool char_array_equal( char* a, char* b, i32 n ){
	for( i32 i = 0; i < n; i++ ){
		if( a[ i ] == 0 || b[ i ] == 0 || a[ i ] != b[ i ] ){
			return false;
		}
	}
	if( a[ n ] == b[ n ] ){
		return false;
	}
	return true;
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
			token.raw = &file->source.data[ file->index ];
			token.length = 1;
			token.column = file->column;
			token.line = file->line;
	                compiler_error_token( file, &token, halt_level, "String literal must not directly follow comment. (Hint: add ' ' after comment)" );
		}
	}
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
		token.length += 1;
                compiler_error_token( file, &token, halt_level, "Unable to parse syntax." );
        }
	return token;
}

void build_node( struct ast_node* node, char* raw, i32 length, i32 line, i32 column, enum ast_node_kind kind ){
	assert( node != NULL, "Malformed argument." );
	node->raw = raw;
	node->length = length;
	node->line = line;
	node->column = column;
	node->kind = kind;
}

enum ast_node_kind parse_expression_token_kind_to_node( struct source_file* file, struct raw_token token ){
	switch( token.kind ){
	case literal_number_token: return literal_number_node;
	case identifier_token:     return register_node;
	case addition_token:       return addition_node;
	case member_token:         return member_node;
	default:                   compiler_error_token( file, &token, error_level, "Expected expression." );
	}
	return 0;
}

bool token_is_expression_leaf( struct raw_token token ){
	return token.kind == literal_number_token || token.kind == identifier_token;
}

bool token_is_binary_operator( struct raw_token token ){
	return token.kind == addition_token || token.kind == member_token;
}

struct ast_node* parse_expression( struct source_file* file, enum raw_token_kind expected_post ){
	assert( file != NULL, "Malformed argument." );
	struct raw_token token = next_token( file );
	if( !token_is_expression_leaf( token )){
		compiler_error_token( file, &token, error_level, "Expected expression leaf." );
	}
	struct ast_node* left = ast_node_array_new( &file->node_raw );
	build_node( left, token.raw, token.length, token.line, token.column, parse_expression_token_kind_to_node( file, token ));
	token = next_token( file );
	if( !token_is_binary_operator( token )){
		if( token.kind == expected_post ){
			return left;
		}
		compiler_error_token( file, &token, error_level, "Expected expression binary operator." );
	}
	struct ast_node* operator = ast_node_array_new( &file->node_raw );
	build_node( operator, token.raw, token.length, token.line, token.column, parse_expression_token_kind_to_node( file, token ));
	token = next_token( file );
	if( !token_is_expression_leaf( token )){
		compiler_error_token( file, &token, error_level, "Expected expression leaf." );
	}
	struct ast_node* right = ast_node_array_new( &file->node_raw );
	build_node( right, token.raw, token.length, token.line, token.column, parse_expression_token_kind_to_node( file, token ));
	operator->sibling = left;
	operator->child = right;
	token = next_token( file );
	if( token.kind == expected_post ){
		return operator;
	} else {
		compiler_error_token( file, &token, error_level, "Expected expression end." );
		return NULL;
	}
}

void parse_jump( struct source_file* file, struct ast_node* local_root ){
	assert( file != NULL, "Malformed argument." );
	assert( local_root != NULL, "Malformed argument." );
	assert( local_root->child == NULL, "Malformed argument data." );
	assert( local_root->kind == jump_node, "Malformed argument data." );
	struct raw_token token = next_token( file );
	if( token.kind != identifier_token ){
		compiler_error_token( file, &token, error_level, "Can only jump to return." );
	}
	if( !char_array_equal( token.raw, "return", 6 )){
		compiler_error_token( file, &token, error_level, "Can only jump to return." );
	}
	build_node( local_root, token.raw, token.length, token.line, token.column, jump_node );
	token = next_token( file );
	if( token.kind != argument_open_token ){
		compiler_error_token( file, &token, error_level, "Expected '[' after jump location." );
	}
	local_root->child = parse_expression( file, argument_close_token );
	token = next_token( file );
	if( token.kind != statement_end_token ){
		compiler_error_token( file, &token, error_level, "';' required at the end of every statement." );
	}
}

void parse_register( struct source_file* file, struct ast_node* local_root ){
	assert( file != NULL, "Malformed argument." );
	assert( local_root != NULL, "Malformed argument." );
	assert( local_root->child == NULL, "Malformed argument data." );
	assert( local_root->kind == register_node, "Malformed argument data." );
	struct raw_token token = next_token( file );
	if( token.kind != identifier_token ){
		compiler_error_token( file, &token, error_level, "regsiter must be followed by type." );
	}
	local_root->child = ast_node_array_new( &file->node_raw );
	build_node( local_root->child, token.raw, token.length, token.line, token.column, type_node );
	token = next_token( file );
	if( token.kind != assignment_token ){
		compiler_error_token( file, &token, error_level, "register must be assigned a value." );
	}
	local_root->child->sibling = parse_expression( file, statement_end_token );
}

void parse_procedure( struct source_file* file, struct ast_node* local_root ){
	assert( file != NULL, "Malformed argument." );
	assert( local_root != NULL, "Malformed argument." );
	assert( local_root->child == NULL, "Malformed argument data." );
	assert( local_root->raw != NULL, "Malformed argument data." );
	assert( local_root->length > 0, "Malformed argument data." );
	assert( local_root->kind == procedure_node, "Malformed argument data." );
	local_root->child = ast_node_array_new( &file->node_raw );
	struct ast_node* routine = local_root->child;
	build_node( routine, local_root->raw, local_root->length, local_root->line, local_root->column, routine_node );
	struct raw_token token = next_token( file );
	if( token.kind != argument_open_token ){
		compiler_error_token( file, &token, error_level, "Expected '[' following procedure declaration." );
	}
	struct ast_node* statement = routine;
	{ // return type
		token = next_token( file );
		if( token.kind != identifier_token ){
			compiler_error_token( file, &token, error_level, "Expected return type to start procedure arguments." );
		}
		statement->child = ast_node_array_new( &file->node_raw );
		statement = statement->child;
		build_node( statement, token.raw, token.length, token.line, token.column, type_node );
		token = next_token( file );
		if( token.kind != result_token ){
			compiler_error_token( file, &token, error_level, "Expected ':' after return type in procedure arguments." );
		}
	}
	{ // arguments
		token = next_token( file );
		if( token.kind != identifier_token ){
			compiler_error_token( file, &token, error_level, "Expected argument name." );
		}
		statement->sibling = ast_node_array_new( &file->node_raw );
		statement = statement->sibling;
		struct ast_node* argument = statement;
		build_node( argument, token.raw, token.length, token.line, token.column, argument_node );
		token = next_token( file );
		if( token.kind != array_token ){
			compiler_error_token( file, &token, error_level, "Start procedure must have one arugment of type '@@i8'." );
		}
		argument->child = ast_node_array_new( &file->node_raw );
		argument = argument->child;
		build_node( argument, token.raw, token.length, token.line, token.column, type_node );
		token = next_token( file );
		if( token.kind != array_token ){
			compiler_error_token( file, &token, error_level, "Start procedure must have one arugment of type '@@i8'." );
		}
		argument->child = ast_node_array_new( &file->node_raw );
		argument = argument->child;
		build_node( argument, token.raw, token.length, token.line, token.column, type_node );
		token = next_token( file );
		if( token.kind != identifier_token ){
			compiler_error_token( file, &token, error_level, "Start procedure must have one arugment of type '@@i8'." );
		}
		argument->child = ast_node_array_new( &file->node_raw );
		argument = argument->child;
		build_node( argument, token.raw, token.length, token.line, token.column, type_node );
		token = next_token( file );
		if( token.kind != argument_close_token ){
			compiler_error_token( file, &token, error_level, "Expected ']' following procedure arguments." );
		}
	}
	token = next_token( file );
	if( token.kind != scope_open_token ){
		compiler_error_token( file, &token, error_level, "Expected '{' following procedue arguments." );
	}
	while( 1 ){
		token = next_token( file );
		if( token.kind == identifier_token ){
			statement->sibling = ast_node_array_new( &file->node_raw );
			statement = statement->sibling;
			build_node( statement, token.raw, token.length, token.line, token.column, register_node );
			parse_register( file, statement );
		} else if( token.kind == jump_token ){
			statement->sibling = ast_node_array_new( &file->node_raw );
			statement = statement->sibling;
			statement->kind = jump_node;
			parse_jump( file, statement );
		} else if( token.kind == scope_close_token ){
			break;
		} else {
			compiler_error_token( file, &token, error_level, "Expected jump or register." );
		}
	}
}

void parse_file( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->child == NULL, "Malformed argument data." );
	assert( root->raw != NULL, "Malformed argument data." );
	assert( root->length > 0, "Malformed argument data." );
	assert( root->kind == file_node, "Malformed argument data." );
	struct source_file file = {
		.node_raw = { 0 },
		.source = { 0 },
		.root = root,
		.index = 0,
		.line = 1,
		.column = 0,
	};
	bool error = string_from_file( &file.source, file.root->raw );
	if( error ){
		compiler_error( &file, file.root, halt_level, "Unable to read from file \"%s\"", root->raw );
	}
	root->child = ast_node_array_new( &file.node_raw );
	struct ast_node* node = root->child;
	struct raw_token token = next_token( &file );
	build_node( node, token.raw, token.length, token.line, token.column, 0 );
	while( 1 ){
		if( token.kind == identifier_token ){
			token = next_token( &file );
			if( token.kind == procedure_token ){
				node->kind = procedure_node;
				if( !char_array_equal( node->raw, "start", 5 )){
					compiler_error( &file, node, error_level, "Compiler only supports start procedure." );
				}
				parse_procedure( &file, node );
			} else {
				compiler_error_token( &file, &token, error_level, "Compiler only supports procedures in global scope." );
			}
		} else {
			compiler_error_token( &file, &token, error_level, "Expected global declaration." );
		}
		token = next_token( &file );
		if( file.index >= file.source.length ){
			break;
		}
		compiler_error_token( &file, &token, error_level, "Compiler only supports start procedure in global scope." );
	}
}

void validate_start_procedure( struct ast_node* local_root ){
	assert( local_root != NULL, "Malformed argument." );
	assert( local_root->raw != NULL, "Malformed argument data." );
	assert( local_root->length > 0, "Malformed argument data." );
	assert( local_root->child != NULL, "Malformed argument data." );
	assert( local_root->kind == procedure_node, "Malformed argument data." );
	assert( char_array_equal( local_root->raw, "start", 5 ), "Malformed argument data." );
	struct ast_node* return_value = local_root->child->child;
	while( return_value->kind != jump_node ){
		assert( return_value->sibling != NULL, "Bug is parse stage." );
		return_value = return_value->sibling;
	}
}

void validate_globals( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->raw != NULL, "Malformed argument data." );
	assert( root->length > 0, "Malformed argument data." );
	assert( root->kind == file_node, "Malformed argument data." );
	assert( root->child != NULL, "Malformed argument data." );
	assert( root->sibling == NULL, "Not supporing multiple files." );
	struct ast_node* node = root->child;
	assert( node->child != NULL, "Malformed argument data." );
	assert( node->sibling == NULL, "Not supporing multiple globals." );
	assert( node->kind == procedure_node, "Malformed argument data." );
	assert( char_array_equal( node->raw, "start", 5 ), "Malformed argument data." );
	validate_start_procedure( node );
}

void output_procedure( struct string* file, struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == procedure_node, "Malformed argument data." );
	string_append( file, "define ", 7 );
	string_append( file, root->child->child->raw, root->child->child->length );
	string_append( file, " @", 2 );
	string_append( file, root->raw, root->length );
	string_append( file, "( ", 2 );
	assert( *root->child->child->sibling->child->raw == '@', "Bad procedure argument for now." );
	assert( *root->child->child->sibling->child->child->raw == '@', "Bad procedure argument for now." );
	string_append( file, "ptr %", 5 );
	string_append( file, root->child->child->sibling->raw, root->child->child->sibling->length );
	string_append( file, ".data, ", 7 );
	string_append( file, "i64 %", 5 );
	string_append( file, root->child->child->sibling->raw, root->child->child->sibling->length );
	string_append( file, ".count ){\n", 10 );
	struct ast_node* statement = root->child->child->sibling->sibling;
	while( 1 ){
		if( statement->kind == register_node ){
			string_append( file, "\t%", 2 );
			string_append( file, statement->raw, statement->length );
			string_append( file, " = add ", 7 );
			string_append( file, statement->child->raw, statement->child->length );
			string_append( file, " ", 1 );
			if( statement->child->sibling->kind == register_node ){
				string_append( file, "%", 1 );
				string_append( file, statement->child->sibling->raw, statement->child->sibling->length );
				string_append( file, ", 0", 3 );
			} else if ( statement->child->sibling->kind == literal_number_node ){
				string_append( file, statement->child->sibling->raw, statement->child->sibling->length );
				string_append( file, ", 0", 3 );
			} else if ( statement->child->sibling->kind == addition_node ){
				if( statement->child->sibling->sibling->kind == register_node ){
					string_append( file, "%", 1 );
					string_append( file, statement->child->sibling->sibling->raw, statement->child->sibling->sibling->length );
				} else if ( statement->child->sibling->sibling->kind == literal_number_node ){
					string_append( file, statement->child->sibling->sibling->raw, statement->child->sibling->sibling->length );
				} else {
					assert( false, "Bad addition statement not valid for now." );
				}
				string_append( file, ", ", 2 );
				if( statement->child->sibling->child->kind == register_node ){
					string_append( file, "%", 1 );
					string_append( file, statement->child->sibling->child->raw, statement->child->sibling->child->length );
				} else if ( statement->child->sibling->child->kind == literal_number_node ){
					string_append( file, statement->child->sibling->child->raw, statement->child->sibling->child->length );
				} else {
					assert( false, "Bad addition statement not valid for now." );
				}
			} else {
				assert( false, "Bad statement not valid for now." );
			}
				string_append( file, "\n", 1 );
		} else if( statement->kind == jump_node ){
			assert( char_array_equal( statement->raw, "return", 6 ), "Bad statement not jump return for now." );
			print_ast();
			if( statement->child->kind == addition_node ){
				string_append( file, "\t%", 2 );
				string_append( file, statement->raw, statement->length );
				string_append( file, ".1 = add ", 9 );
				string_append( file, root->child->child->raw, root->child->child->length );
				string_append( file, " ", 1 );
				if( statement->child->sibling->kind == register_node ){
					string_append( file, "%", 1 );
					string_append( file, statement->child->sibling->raw, statement->child->sibling->length );
				} else if ( statement->child->sibling->kind == literal_number_node ){
					string_append( file, statement->child->sibling->raw, statement->child->sibling->length );
				} else {
					assert( false, "Bad addition statement not valid for now." );
				}
				string_append( file, ", ", 2 );
				if( statement->child->child->kind == register_node ){
					string_append( file, "%", 1 );
					string_append( file, statement->child->child->raw, statement->child->child->length );
				} else if ( statement->child->child->kind == literal_number_node ){
					string_append( file, statement->child->child->raw, statement->child->child->length );
				} else {
					assert( false, "Bad addition statement not valid for now." );
				}
				string_append( file, "\n\tret ", 6 );
				string_append( file, root->child->child->raw, root->child->child->length );
				string_append( file, " %", 2 );
				string_append( file, statement->raw, statement->length );
				string_append( file, ".1", 2 );
			} else if( statement->child->kind == member_node ){
				assert( char_array_equal( statement->child->sibling->raw, "argument", 8 ), "Only supporting argument.count member." );
				assert( char_array_equal( statement->child->child->raw, "count", 5 ), "Only supporting argument.count member." );
				string_append( file, "\tret ", 5 );
				string_append( file, root->child->child->raw, root->child->child->length );
				string_append( file, " %argument.count", 16 );
			} else {
				string_append( file, "\tret ", 5 );
				string_append( file, root->child->child->raw, root->child->child->length );
				string_append( file, " ", 1 );
				if( statement->child->kind == register_node ){
					string_append( file, "%", 1 );
					string_append( file, statement->child->raw, statement->child->length );
				} else if ( statement->child->kind == literal_number_node ){
					string_append( file, statement->child->raw, statement->child->length );
				} else {
					assert( false, "Bad statement not valid return for now." );
				}
			}
			string_append( file, "\n", 1 );
			break;
		} else {
			assert( false, "Bad statement not jump for now." );
		}
		assert( statement->sibling != NULL, "Invalid ast." );
		statement = statement->sibling;
	}
	string_append( file, "}\n", 2 );
}

void output( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == file_node, "Malformed argument data." );
	struct string file = { 0 };
	char llvmir_start[] = "target triple = \"x86_64-unknown-linux-gnu\"\n\n";
	string_append( &file, llvmir_start, sizeof( llvmir_start ) - 1 );
	output_procedure( &file, root->child );
	root->raw[ root->length - 2 ] = 'l';
	bool error = string_to_file( &file, root->raw );
	if( error ){
		printf( "%s%sError%s Unable to write to file \"%s\"%s\n\n", ansi_bold_start, ansi_foreground_red, ansi_foreground_default, root->raw, ansi_bold_end );
	}
}

i32 main( i32 argc, char* argv[] ){
	if( argc != 2 ){
		fprintf( stderr, "Usage: ssalc file_name.sl\n" );
		exit( 1 );
	}
	root_node.raw = argv[ 1 ],
	root_node.length = strlen( argv[ 1 ]),
	root_node.kind = file_node,
	parse_file( &root_node );
	validate_globals( &root_node );
	output( &root_node );
#ifdef DEBUG
	print_ast();
#endif
	return 0;
}

