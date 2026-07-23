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
	list_separator_node,
	literal_number_node,
	literal_string_node,
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

struct ast_node_pointer_array {
	struct ast_node** data;
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

struct output_context {
	struct string file;
	struct string literal_string;
	i32 literal_string_number;
};

enum compiler_error_level {
	halt_level,
	error_level,
	warning_level,
	note_level,
};

#include "array.h"


struct output_context output = { 0 };
struct ast_node root_node = { 0 };

char* node_kind_string( struct ast_node* node ){
	switch( node->kind ){
		case error_node:          return "error";
		case file_node:           return "file";
		case procedure_node:      return "procedure";
		case routine_node:        return "routine";
		case structure_node:      return "structure";
		case union_node:          return "union";
		case enumeration_node:    return "enumeration";
		case argument_node:       return "argument";
		case register_node:       return "register";
		case type_node:           return "type";
		case jump_node:           return "jump";
		case call_node:           return "call";
		case list_separator_node: return "list_separator";
		case literal_number_node: return "literal_number";
		case literal_string_node: return "literal_string";
		case member_node:         return "member";
		case addition_node:       return "addition";
		default:                  assert( false, "Unknown node kind" );
	}
}

void print_ast_node( struct ast_node* node, i32 depth ){
        assert( node != NULL, "Malformed argument." );
	char* kind = node_kind_string( node );
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
		case literal_string_token: return literal_string_node;
		case identifier_token:     return register_node;
		case addition_token:       return addition_node;
		case member_token:         return member_node;
		case list_separator_token: return list_separator_node;
		default:                   compiler_error_token( file, &token, error_level, "Expected expression." );
	}
	return 0;
}

bool token_is_expression_leaf( struct raw_token token ){
	return token.kind == identifier_token ||
	       token.kind == literal_number_token ||
	       token.kind == literal_string_token;
}

bool token_is_binary_operator( struct raw_token token ){
	return token.kind == addition_token ||
	       token.kind == member_token ||
	       token.kind == list_separator_token;
}

struct ast_node* parse_expression( struct source_file* file, enum raw_token_kind expected_post ){
	assert( file != NULL, "Malformed argument." );
	struct raw_token token = next_token( file );
	if( !token_is_expression_leaf( token )){
		compiler_error_token( file, &token, error_level, "Expected expression leaf." );
	}
	struct ast_node* left = ast_node_array_new( &file->node_raw );
	struct ast_node* operator = { 0 };
	struct ast_node* right = { 0 };
	build_node( left, token.raw, token.length, token.line, token.column, parse_expression_token_kind_to_node( file, token ));
	token = next_token( file );
	if( !token_is_binary_operator( token )){
		if( token.kind == argument_open_token ){
			operator = left;
			operator->sibling = parse_expression( file, argument_close_token );
			operator->kind = call_node;
			token = next_token( file );
			if( token.kind == expected_post ){
				return operator;
			}
			compiler_error_token( file, &token, error_level, "Expected expression binary operator or end." );
		}
		if( token.kind == expected_post ){
			return left;
		}
		compiler_error_token( file, &token, error_level, "Expected expression binary operator." );
	}
	while( 1 ){
		operator = ast_node_array_new( &file->node_raw );
		build_node( operator, token.raw, token.length, token.line, token.column, parse_expression_token_kind_to_node( file, token ));
		token = next_token( file );
		if( !token_is_expression_leaf( token )){
			compiler_error_token( file, &token, error_level, "Expected expression leaf." );
		}
		right = ast_node_array_new( &file->node_raw );
		build_node( right, token.raw, token.length, token.line, token.column, parse_expression_token_kind_to_node( file, token ));
		operator->sibling = left;
		operator->child = right;
		token = next_token( file );
		if( !token_is_binary_operator( token )){
			if( token.kind == expected_post ){
				return operator;
			}
			compiler_error_token( file, &token, error_level, "Expected expression binary operator or end." );
		}
		left = operator;
		operator = NULL;
		right = NULL;
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

i32 output_add_string( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == literal_string_node, "Malformed argument." );
	string_append( &output.literal_string, "@.literal_string.", 17 );
	string_alloc( &output.literal_string, 32 );
	i32 string_added = sprintf( &output.literal_string.data[ output.literal_string.length ], "%d", output.literal_string_number );
	output.literal_string.length += string_added;
	string_append( &output.literal_string, " = global [ ", 12 );
	string_alloc( &output.literal_string, 32 );
	string_added = sprintf( &output.literal_string.data[ output.literal_string.length ], "%d", root->length - 2 );
	output.literal_string.length += string_added;
	string_append( &output.literal_string, " x i8 ] c", 9 );
	string_append( &output.literal_string, root->raw, root->length );
	output.literal_string_number += 1;
	return output.literal_string_number - 1;
}

void output_procedure_call( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == call_node, "Malformed argument." );
	string_append( &output.file, "call ", 5 );
	if( char_array_equal( root->raw, "write_syscall", 13 )){
		string_append( &output.file, "i64 @write_syscall( i64 ", 24 );
		string_append( &output.file, root->sibling->sibling->sibling->raw, root->sibling->sibling->sibling->length );
		string_append( &output.file, ", ptr ", 6 );
		string_append( &output.file, "@.literal_string.", 17 );
		i32 index = output_add_string( root->sibling->sibling->child );
		string_alloc( &output.file, 32 );
		i32 string_added = sprintf( &output.file.data[ output.file.length ], "%d", index );
		output.file.length += string_added;
		string_append( &output.file, ", i64 ", 6 );
		string_append( &output.file, root->sibling->child->raw, root->sibling->child->length );
		string_append( &output.file, " )", 2 );
	} else {
		assert( false, "Not supporting arbitrary procedure calls yet." );
	}
}

void output_register( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == register_node, "Malformed argument." );
	string_append( &output.file, "\t%", 2 );
	string_append( &output.file, root->raw, root->length );
	string_append( &output.file, " = ", 3 );
	struct ast_node* value = root->child->sibling;
	if( value->kind == register_node ){
		string_append( &output.file, "add ", 4 );
		string_append( &output.file, root->child->raw, root->child->length );
		string_append( &output.file, " %", 2 );
		string_append( &output.file, value->raw, value->length );
		string_append( &output.file, ", 0", 3 );
	} else if ( value->kind == literal_number_node ){
		string_append( &output.file, "add ", 4 );
		string_append( &output.file, root->child->raw, root->child->length );
		string_append( &output.file, " ", 1 );
		string_append( &output.file, value->raw, value->length );
		string_append( &output.file, ", 0", 3 );
	} else if ( value->kind == addition_node ){
		string_append( &output.file, "add ", 4 );
		string_append( &output.file, root->child->raw, root->child->length );
		string_append( &output.file, " ", 1 );
		if( value->sibling->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, value->sibling->raw, value->sibling->length );
		} else if ( value->sibling->kind == literal_number_node ){
			string_append( &output.file, value->sibling->raw, value->sibling->length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
		string_append( &output.file, ", ", 2 );
		if( value->child->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, value->child->raw, value->child->length );
		} else if ( value->child->kind == literal_number_node ){
			string_append( &output.file, value->child->raw, value->child->length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
	} else if ( value->kind == call_node ){
		output_procedure_call( value );
	} else {
		assert( false, "Bad root not valid for now." );
	}
	string_append( &output.file, "\n", 1 );
}

void output_jump( struct ast_node* root, struct ast_node* procedure ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == jump_node, "Malformed argument." );
	assert( procedure != NULL, "Malformed argument." );
	assert( procedure->kind == procedure_node, "Malformed argument data." );
	assert( char_array_equal( root->raw, "return", 6 ), "Bad root not jump return for now." );
	if( root->child->kind == addition_node ){
		string_append( &output.file, "\t%", 2 );
		string_append( &output.file, root->raw, root->length );
		string_append( &output.file, ".1 = add ", 9 );
		string_append( &output.file, procedure->child->child->raw, procedure->child->child->length );
		string_append( &output.file, " ", 1 );
		if( root->child->sibling->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, root->child->sibling->raw, root->child->sibling->length );
		} else if ( root->child->sibling->kind == literal_number_node ){
			string_append( &output.file, root->child->sibling->raw, root->child->sibling->length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
		string_append( &output.file, ", ", 2 );
		if( root->child->child->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, root->child->child->raw, root->child->child->length );
		} else if ( root->child->child->kind == literal_number_node ){
			string_append( &output.file, root->child->child->raw, root->child->child->length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
		string_append( &output.file, "\n\tret ", 6 );
		string_append( &output.file, procedure->child->child->raw, procedure->child->child->length );
		string_append( &output.file, " %", 2 );
		string_append( &output.file, root->raw, root->length );
		string_append( &output.file, ".1", 2 );
	} else if( root->child->kind == member_node ){
		assert( char_array_equal( root->child->sibling->raw, "argument", 8 ), "Only supporting argument.count member." );
		assert( char_array_equal( root->child->child->raw, "count", 5 ), "Only supporting argument.count member." );
		string_append( &output.file, "\tret ", 5 );
		string_append( &output.file, procedure->child->child->raw, procedure->child->child->length );
		string_append( &output.file, " %argument.count", 16 );
	} else {
		string_append( &output.file, "\tret ", 5 );
		string_append( &output.file, procedure->child->child->raw, procedure->child->child->length );
		string_append( &output.file, " ", 1 );
		if( root->child->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, root->child->raw, root->child->length );
		} else if ( root->child->kind == literal_number_node ){
			string_append( &output.file, root->child->raw, root->child->length );
		} else {
			assert( false, "Bad root not valid return for now." );
		}
	}
	string_append( &output.file, "\n", 1 );
}

void output_procedure( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == procedure_node, "Malformed argument data." );
	string_append( &output.file, "define ", 7 );
	string_append( &output.file, root->child->child->raw, root->child->child->length );
	string_append( &output.file, " @", 2 );
	string_append( &output.file, root->raw, root->length );
	string_append( &output.file, "( ", 2 );
	assert( *root->child->child->sibling->child->raw == '@', "Bad procedure argument for now." );
	assert( *root->child->child->sibling->child->child->raw == '@', "Bad procedure argument for now." );
	string_append( &output.file, "ptr %", 5 );
	string_append( &output.file, root->child->child->sibling->raw, root->child->child->sibling->length );
	string_append( &output.file, ".data, ", 7 );
	string_append( &output.file, "i64 %", 5 );
	string_append( &output.file, root->child->child->sibling->raw, root->child->child->sibling->length );
	string_append( &output.file, ".count ){\n", 10 );
	struct ast_node* statement = root->child->child->sibling->sibling;
	while( 1 ){
		if( statement->kind == register_node ){
			output_register( statement );
		} else if( statement->kind == jump_node ){
			output_jump( statement, root );
			break;
		} else {
			assert( false, "Bad statement not jump for now." );
		}
		assert( statement->sibling != NULL, "Invalid ast." );
		statement = statement->sibling;
	}
	string_append( &output.file, "}\n", 2 );
}

void output_llvm( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == file_node, "Malformed argument data." );
	char llvmir_start[] = "target triple = \"x86_64-unknown-linux-gnu\"\n\ndeclare i64 @write_syscall( i64, ptr, i64 )\n\n";
	string_append( &output.file, llvmir_start, sizeof( llvmir_start ) - 1 );
	output_procedure( root->child );
	string_append( &output.file, "\n", 1 );
	if( output.literal_string.length > 0 ){
		string_append( &output.file, output.literal_string.data, output.literal_string.length );
		string_append( &output.file, "\n", 1 );
	}
	root->raw[ root->length - 2 ] = 'l';
	bool error = string_to_file( &output.file, root->raw );
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
	output_llvm( &root_node );
#ifdef DEBUG
	print_ast();
#endif
	return 0;
}

