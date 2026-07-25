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
	procedure_call_node,
	list_separator_node,
	literal_number_node,
	literal_string_node,
	member_node,
	addition_node,
	subtraction_node,
	multiplication_node,
	division_node,
	modulo_node,
	shift_right_node,
	shift_left_node,
	bit_not_node,
	bit_and_node,
	bit_or_node,
	bit_xor_node,
	logical_not_node,
	logical_and_node,
	logical_or_node,
	equal_node,
	less_node,
	greater_node,
	less_equal_node,
	greater_equal_node,
	less_greater_node,
};

struct raw_token {
	char* file;
	char* raw;
	i32 file_length;
	i32 length;
	i32 line;
	i32 column;
	enum raw_token_kind kind;
};

struct ast_node {
	struct ast_node* sibling;
	struct ast_node* child;
	struct raw_token token;
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
	char* name;
	i32 name_length;
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
	error_level,
	warning_level,
	note_level,
};

#include "array.h"

struct ast_node root_node = { 0 };
struct output_context output = { 0 };

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
		case procedure_call_node: return "call";
		case list_separator_node: return "list_separator";
		case literal_number_node: return "literal_number";
		case literal_string_node: return "literal_string";
		case member_node:         return "member";
		case addition_node:       return "addition";
		case subtraction_node:    return "subtraction";
		case multiplication_node: return "multiplication";
		case division_node:       return "division";
		case modulo_node:         return "modulo";
		case shift_right_node:    return "shift_right";
		case shift_left_node:     return "shift_left";
		case bit_not_node:        return "bit_not";
		case bit_and_node:        return "bit_and";
		case bit_or_node:         return "bit_or";
		case bit_xor_node:        return "bit_xor";
		case logical_not_node:    return "logical_not";
		case logical_and_node:    return "logical_and";
		case logical_or_node:     return "logical_or";
		case equal_node:          return "equal";
		case less_node:           return "less";
		case greater_node:        return "greater";
		case less_equal_node:     return "less_equal";
		case greater_equal_node:  return "greater_equal";
		case less_greater_node:   return "less_greater";
		default:                  assert( false, "Unknown node kind" );
	}
}

void print_ast_node( struct ast_node* node, i32 depth ){
        assert( node != NULL, "Malformed argument." );
	char* kind = node_kind_string( node );
        printf( "%*c[ %s%s%s ]  %.*s\n", depth, ' ', ansi_foreground_cyan, kind, ansi_reset_graphics, node->token.length, node->token.raw );
        if( node->child != NULL ){
                print_ast_node( node->child, depth + 1 );
        }
        if( node->sibling != NULL ){
                print_ast_node( node->sibling, depth );
        }
}

void print_ast(){
	printf( "\n=== Ast Start ===\n\n" );
        print_ast_node( &root_node, 1 );
	printf( "\n==== Ast End ====\n\n" );
}

void compiler_error( struct raw_token problem, enum compiler_error_level level, char* format, ... ){
        assert( problem.file != NULL, "Malformed argument." );
        assert( problem.raw != NULL, "Malformed argument." );
        assert( problem.length > 0, "Malformed argument." );
        assert( problem.column >= 0, "Malformed argument." );
        assert( problem.line >= 0, "Malformed argument." );
        assert( format != NULL, "Malformed argument." );
	if( level == error_level ){
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
	printf( "%s\n", ansi_bold_end );
	char* start = problem.raw - problem.column;
	i32 bytes_after_problem = 0;
	while(( problem.raw[ bytes_after_problem + problem.length ] != '\n' ) &&
	      ( problem.raw[ bytes_after_problem + problem.length ] != '\r' ) &&
	      ( problem.raw[ bytes_after_problem + problem.length ] != '\0' )){
		bytes_after_problem += 1;
	}
	printf( "  %.*s | %d | %.*s%s%.*s%s%.*s\n\n",
		problem.file_length, problem.file, problem.line,
		problem.column, start, ansi_underline_start ansi_foreground_red,
	        problem.length, start + problem.column, ansi_reset_graphics,
		bytes_after_problem, start + problem.column + problem.length
	);
#ifdef DEBUG
	print_ast();
#endif
	exit( 1 );
}

#define compiler_assert( expr, problem, level, ... ){ \
	if( !( expr )){ \
		compiler_error( problem, level, __VA_ARGS__ ); \
	} \
}

bool char_array_equal( char* a, char* b, i32 n ){
	assert( a != NULL, "Malformed argument." );
	assert( b != NULL, "Malformed argument." );
	assert( n > 0, "Malformed argument." );
	for( i32 i = 0; i < n; i++ ){
		if( a[ i ] == 0 || b[ i ] == 0 || a[ i ] != b[ i ] ){
			return false;
		}
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
	assert( file->name != NULL, "Malformed argument data." );
	assert( file->name_length > 0, "Malformed argument data." );
	struct raw_token token = { 0 };
	token.file = file->name;
	token.file_length = file->name_length;
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
	if(( '\\' == file->source.data[ file->index ]) && ( '{' == file->source.data[ file->index + 1 ])){
		bool in_string = false;
		i32 nesting = 0;
		while( 1 ){
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
			if(( '\\' == file->source.data[ file->index ]) && ( '{' == file->source.data[ file->index + 1 ])){
				if( in_string == false ){
					nesting += 1;
				}
			}
			if(( in_string == false ) && ( '}' == file->source.data[ file->index ]) && ( '\\' == file->source.data[ file->index + 1 ])){
				if( in_string == false ){
					if( nesting == 0 ){
						break;
					}
					nesting -= 1;
				}
			}
		}
		file->column += 2;
		file->index += 2;
		if( '"' == file->source.data[ file->index ]){
			token.raw = &file->source.data[ file->index ];
			token.length = 1;
			token.column = file->column;
			token.line = file->line;
	                compiler_error( token, error_level, "String literal must not directly follow comment. (Hint: add ' ' after comment)" );
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
                compiler_error( token, error_level, "Unable to parse syntax." );
        }
	return token;
}

enum ast_node_kind parse_expression_token_kind_to_node( struct raw_token token ){
	switch( token.kind ){
		case literal_number_token: return literal_number_node;
		case literal_string_token: return literal_string_node;
		case identifier_token:     return register_node;
		case addition_token:       return addition_node;
		case member_token:         return member_node;
		case list_separator_token: return list_separator_node;
		default:                   assert( false, "Expected expression." );
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
	compiler_assert( token_is_expression_leaf( token ), token, error_level, "Expected register or literal at the start of an expression." );
	struct ast_node* left = ast_node_array_new( &file->node_raw );
	struct ast_node* operator = { 0 };
	struct ast_node* right = { 0 };
	left->token = token;
	left->kind = parse_expression_token_kind_to_node( token );
	token = next_token( file );
	if( !token_is_binary_operator( token )){
		if( token.kind == argument_open_token ){
			operator = left;
			operator->sibling = parse_expression( file, argument_close_token );
			operator->kind = procedure_call_node;
			token = next_token( file );
			if( token.kind == expected_post ){
				return operator;
			}
			compiler_error( token, error_level, "Expected expression binary operator or end." );
		}
		if( token.kind == expected_post ){
			return left;
		}
		compiler_error( token, error_level, "Expected expression binary operator." );
	}
	while( 1 ){
		operator = ast_node_array_new( &file->node_raw );
		operator->token = token;
		operator->kind = parse_expression_token_kind_to_node( token );
		token = next_token( file );
		compiler_assert( token_is_expression_leaf( token ), token, error_level, "Expected expression leaf." );
		right = ast_node_array_new( &file->node_raw );
		right->token = token;
		right->kind = parse_expression_token_kind_to_node( token );
		operator->sibling = left;
		operator->child = right;
		token = next_token( file );
		if( !token_is_binary_operator( token )){
			if( token.kind == expected_post ){
				return operator;
			}
			compiler_error( token, error_level, "Expected expression binary operator or end." );
		}
		left = operator;
		operator = NULL;
		right = NULL;
	}
}

void parse_jump( struct source_file* file, struct ast_node* root ){
	assert( file != NULL, "Malformed argument." );
	assert( root != NULL, "Malformed argument." );
	assert( root->child == NULL, "Malformed argument data." );
	assert( root->kind == jump_node, "Malformed argument data." );
	struct raw_token token = next_token( file );
	compiler_assert( token.kind == identifier_token, token, error_level, "Can only jump to return." );
	compiler_assert( char_array_equal( token.raw, "return", 6 ), token, error_level, "Can only jump to return." );
	root->token = token;
	root->kind = jump_node;
	token = next_token( file );
	compiler_assert( token.kind == argument_open_token, token, error_level, "Expected '[' after jump location." );
	root->child = parse_expression( file, argument_close_token );
	token = next_token( file );
	compiler_assert( token.kind == statement_end_token, token, error_level, "';' required at the end of every statement." );
}

void parse_register( struct source_file* file, struct ast_node* root ){
	assert( file != NULL, "Malformed argument." );
	assert( root != NULL, "Malformed argument." );
	assert( root->child == NULL, "Malformed argument data." );
	assert( root->kind == register_node, "Malformed argument data." );
	struct raw_token token = next_token( file );
	compiler_assert( token.kind == identifier_token, token, error_level, "regsiter must be followed by type." );
	root->child = ast_node_array_new( &file->node_raw );
	root->child->token = token;
	root->child->kind = type_node;
	token = next_token( file );
	compiler_assert( token.kind == assignment_token, token, error_level, "register must be assigned a value." );
	root->child->sibling = parse_expression( file, statement_end_token );
}

void parse_procedure( struct source_file* file, struct ast_node* root ){
	assert( file != NULL, "Malformed argument." );
	assert( root != NULL, "Malformed argument." );
	assert( root->child == NULL, "Malformed argument data." );
	assert( root->kind == procedure_node, "Malformed argument data." );
	root->child = ast_node_array_new( &file->node_raw );
	struct ast_node* routine = root->child;
	routine->token = root->token;
	routine->kind = routine_node;
	struct raw_token token = next_token( file );
	compiler_assert( token.kind == argument_open_token, token, error_level, "Expected '[' following procedure declaration." );
	struct ast_node* statement = routine;
	{ // return type
		token = next_token( file );
		compiler_assert( token.kind == identifier_token, token, error_level, "Expected return type to start procedure arguments." );
		statement->child = ast_node_array_new( &file->node_raw );
		statement = statement->child;
		statement->token = token;
		statement->kind = type_node;
		token = next_token( file );
		compiler_assert( token.kind == result_token, token, error_level, "Expected ':' after return type in procedure arguments." );
	}
	{ // arguments
		token = next_token( file );
		compiler_assert( token.kind == identifier_token, token, error_level, "Expected argument name." );
		statement->sibling = ast_node_array_new( &file->node_raw );
		statement = statement->sibling;
		struct ast_node* argument = statement;
		argument->token = token;
		argument->kind = argument_node;
		token = next_token( file );
		compiler_assert( token.kind == array_token, token, error_level, "Start procedure must have one arugment of type '@@i8'." );
		argument->child = ast_node_array_new( &file->node_raw );
		argument = argument->child;
		argument->token = token;
		argument->kind = type_node;
		token = next_token( file );
		compiler_assert( token.kind == array_token, token, error_level, "Start procedure must have one arugment of type '@@i8'." );
		argument->child = ast_node_array_new( &file->node_raw );
		argument = argument->child;
		argument->token = token;
		argument->kind = type_node;
		token = next_token( file );
		compiler_assert( token.kind == identifier_token, token, error_level, "Start procedure must have one arugment of type '@@i8'." );
		argument->child = ast_node_array_new( &file->node_raw );
		argument = argument->child;
		argument->token = token;
		argument->kind = type_node;
		token = next_token( file );
		compiler_assert( token.kind == argument_close_token, token, error_level, "Expected ']' following procedure arguments." );
	}
	token = next_token( file );
	compiler_assert( token.kind == scope_open_token, token, error_level, "Expected '{' following procedue arguments." );
	while( 1 ){
		token = next_token( file );
		if( token.kind == identifier_token ){
			statement->sibling = ast_node_array_new( &file->node_raw );
			statement = statement->sibling;
			statement->token = token;
			statement->kind = register_node;
			parse_register( file, statement );
		} else if( token.kind == jump_token ){
			statement->sibling = ast_node_array_new( &file->node_raw );
			statement = statement->sibling;
			statement->kind = jump_node;
			parse_jump( file, statement );
		} else if( token.kind == scope_close_token ){
			break;
		} else {
			compiler_error( token, error_level, "Expected jump or register." );
		}
	}
}

void parse_file( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->child == NULL, "Malformed argument data." );
	assert( root->token.raw != NULL, "Malformed argument data." );
	assert( root->token.length > 0, "Malformed argument data." );
	assert( root->kind == file_node, "Malformed argument data." );
	struct source_file file = {
		.node_raw = { 0 },
		.source = { 0 },
		.root = root,
		.name = root->token.raw,
		.name_length = root->token.file_length,
		.index = 0,
		.line = 1,
		.column = 0,
	};
	bool error = string_from_file( &file.source, root->token.raw );
	compiler_assert( !error, root->token, error_level, "Unable to read from file \"%s\"", root->token.raw );
	root->child = ast_node_array_new( &file.node_raw );
	struct ast_node* node = root->child;
	struct raw_token token = next_token( &file );
	node->token = token;
	while( 1 ){
		if( token.kind == identifier_token ){
			token = next_token( &file );
			if( token.kind == procedure_token ){
				node->kind = procedure_node;
				compiler_assert( char_array_equal( node->token.raw, "start", 5 ), node->token, error_level, "Compiler only supports start procedure." );
				parse_procedure( &file, node );
			} else {
				compiler_error( token, error_level, "Compiler only supports procedures in global scope." );
			}
		} else {
			compiler_error( token, error_level, "Expected global declaration." );
		}
		token = next_token( &file );
		if( file.index >= file.source.length ){
			break;
		}
		compiler_error( token, error_level, "Compiler only supports start procedure in global scope." );
	}
}

bool node_is_operator( struct ast_node* node ){
	return node->kind == addition_node ||
	       node->kind == member_node;
}

void add_avalible_register( struct ast_node_pointer_array* available_register, struct ast_node* node ){
	assert( available_register != NULL, "Malformed argument." );
	assert( node != NULL, "Malformed argument." );
	struct ast_node** ptr = ast_node_pointer_array_new( available_register );
	*ptr = node;
}

void validate_register_type( struct ast_node* type, struct ast_node* value, struct ast_node_pointer_array* available_register ){
	assert( type != NULL, "Malformed argument." );
	assert( value != NULL, "Malformed argument." );
	assert( available_register != NULL, "Malformed argument." );
	struct ast_node** result = ast_node_pointer_array_search_derefrence( available_register, value );
	compiler_assert( result != NULL, value->token, error_level, "Register not declared." );
	compiler_assert( type->token.length = (*result)->child->token.length, value->token, error_level, "Register value is diffrent type." );
	bool types_equal = char_array_equal( type->token.raw, (*result)->child->token.raw, type->token.length );
	compiler_assert( types_equal, value->token, error_level, "Register value is diffrent type." );
}

void validate_expression( struct ast_node* expression, struct ast_node* type, struct ast_node_pointer_array* available_register ){
	assert( expression != NULL, "Malformed argument." );
	assert( node_is_operator( expression ), "Malformed argument." );
	assert( type != NULL, "Malformed argument." );
	assert( type->kind == type_node, "Malformed argument." );
	assert( available_register != NULL, "Malformed argument." );
	if( expression->kind == member_node ){
		struct ast_node** result = ast_node_pointer_array_search_derefrence( available_register, expression->sibling );
		compiler_assert( result != NULL, expression->sibling->token, error_level, "Register not declared." );
		compiler_assert( char_array_equal( (*result)->child->token.raw, "@", 1 ), expression->token, error_level, "Register value is diffrent type." );
		compiler_assert( char_array_equal( (*result)->child->child->token.raw, "@", 1 ), expression->token, error_level, "Register value is diffrent type." );
		compiler_assert( char_array_equal( (*result)->child->child->child->token.raw, "i8", 2 ), expression->token, error_level, "Register value is diffrent type." );
		if( char_array_equal( expression->child->token.raw, "data", 4 )){
			assert( 0, "Unreachable" );
		} else if( char_array_equal( expression->child->token.raw, "count", 5 )){
			compiler_assert( char_array_equal( type->token.raw, "i64", 3 ), expression->child->token, error_level, "Register value is different type." );
		} else {
			compiler_error( expression->child->token, error_level, "Register value is different type." );
		}
		return;
	}
	if( node_is_operator( expression->sibling )){
		validate_expression( expression->sibling, type, available_register );
	} else if( expression->sibling->kind == register_node ){
		validate_register_type( type, expression->sibling, available_register );
	} else if( expression->sibling->kind == literal_number_node ){
		// overflow warning
	} else {
		compiler_error( expression->sibling->token, error_level, "Invalid operator opperand." );
	}
	if( node_is_operator( expression->child )){
		validate_expression( expression->child, type, available_register );
	} else if( expression->child->kind == register_node ){
		validate_register_type( type, expression->child, available_register );
	} else if( expression->child->kind == literal_number_node ){
		// overflow warning
	} else {
		compiler_error( expression->child->token, error_level, "Invalid operator opperand." );
	}
}

void validate_routine( struct ast_node* routine, struct ast_node* return_type, struct ast_node_pointer_array available_register ){
	assert( routine != NULL, "Malformed argument." );
	assert( routine->child != NULL, "Malformed argument data." );
	assert( routine->kind == routine_node, "Malformed argument data." );
	assert( return_type != NULL, "Malformed argument." );
	assert( return_type->kind == type_node, "Malformed argument data." );
	compiler_assert( routine->child == return_type, return_type->token, error_level, "Temp, only one routine, existing return type." );
	struct ast_node* argument = routine->child->sibling;
	compiler_assert( argument->kind == argument_node, argument->token, error_level, "Temp, only one argument." );
	add_avalible_register( &available_register, argument );
	struct ast_node* statement = argument->sibling;
	while( 1 ){
		if( statement->kind == jump_node ){
			if( node_is_operator( statement->child )){
				validate_expression( statement->child, return_type, &available_register );
			} else if( statement->child->kind == register_node ){
				validate_register_type( return_type, statement->child, &available_register );
			} else if( statement->child->kind == literal_number_node ){
				// overflow warning
			} else {
				compiler_error( statement->child->token, error_level, "Return type does not match." );
			}
			break;
		} else if( statement->kind == register_node ){
			struct ast_node* register_type = statement->child;
			compiler_assert( register_type->kind == type_node, register_type->token, error_level, "Register must have type." );
			struct ast_node* value = statement->child->sibling;
			if( node_is_operator( value )){
				validate_expression( value, register_type, &available_register );
			} else if( value->kind == procedure_call_node ){
				compiler_assert( char_array_equal( value->token.raw, "write_syscall", 13 ), value->token, error_level, "Procedure not defined." );
				compiler_assert( value->sibling->kind == list_separator_node, value->token, error_level, "Procedure signature does not match." );
				compiler_assert( value->sibling->child->kind == literal_string_node, value->token, error_level, "Procedure signature does not match." );
				if( value->sibling->sibling->kind == literal_number_node ){
					// overflow warning
				} else if( value->sibling->sibling->kind == register_node ){
					validate_register_type( register_type, value->sibling->sibling, &available_register );
				} else {
					compiler_error( value->token, error_level, "Procedure signature does not match." );
				}
			} else if( value->kind == register_node ){
				validate_register_type( register_type, value, &available_register );
			} else if( value->kind == literal_number_node ){
				// overflow warning
			} else {
				compiler_error( value->token, error_level, "Unexpected register value." );
			}
			add_avalible_register( &available_register, statement );
		}
		assert( statement->sibling != NULL, "Parser error." );
		statement = statement->sibling;
	}
	free( available_register.data );
}

void validate_procedure_body( struct ast_node* procedure ){
	assert( procedure != NULL, "Malformed argument." );
	assert( procedure->token.raw != NULL, "Malformed argument data." );
	assert( procedure->token.length > 0, "Malformed argument data." );
	assert( procedure->child != NULL, "Malformed argument data." );
	assert( procedure->kind == procedure_node, "Malformed argument data." );
	struct ast_node* first_routine = procedure->child;
	compiler_assert( first_routine->kind == routine_node, first_routine->token, error_level, "Temp: procedure must have one return value." );
	struct ast_node* return_type = first_routine->child;
	compiler_assert( return_type->kind == type_node, return_type->token, error_level, "Temp: procedure must have one return value." );
	struct ast_node_pointer_array available_register = { 0 };
	validate_routine( first_routine, return_type, available_register );
}

void validate_start_procedure( struct ast_node* procedure ){
	assert( procedure != NULL, "Malformed argument." );
	assert( procedure->token.raw != NULL, "Malformed argument data." );
	assert( procedure->token.length > 0, "Malformed argument data." );
	assert( procedure->child != NULL, "Malformed argument data." );
	assert( procedure->kind == procedure_node, "Malformed argument data." );
	assert( char_array_equal( procedure->token.raw, "start", 5 ), "Malformed argument data." );
	struct ast_node* start_routine = procedure->child;
	assert( start_routine->child != NULL, "Malformed argument data." );
	assert( start_routine->kind == routine_node, "Malformed argument data." );
	assert( char_array_equal( start_routine->token.raw, "start", 5 ), "Malformed argument data." );
	struct ast_node* procedure_return = start_routine->child;
	compiler_assert( procedure_return->kind == type_node, procedure_return->token,  error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( char_array_equal( procedure_return->token.raw, "i64", 3 ), procedure_return->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( procedure_return->sibling != NULL, procedure_return->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	struct ast_node* argument = procedure_return->sibling;
	compiler_assert( argument->kind == argument_node, argument->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( argument->child != NULL, argument->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	struct ast_node* argument_type = argument->child;
	compiler_assert( argument_type->kind == type_node, argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( char_array_equal( argument_type->token.raw, "@", 1 ), argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( argument_type->child != NULL, argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	argument_type = argument_type->child;
	compiler_assert( argument_type->kind == type_node, argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( char_array_equal( argument_type->token.raw, "@", 1 ), argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( argument_type->child != NULL, argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	argument_type = argument_type->child;
	compiler_assert( argument_type->kind == type_node, argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( char_array_equal( argument_type->token.raw, "i8", 2 ), argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	compiler_assert( argument_type->child == NULL, argument_type->token, error_level, 
	                 "'start' procedure must have signatrue 'start procedure[ i64 : argument @@i8 ]'." );
	validate_procedure_body( procedure );
}

void validate_globals( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->token.raw != NULL, "Malformed argument data." );
	assert( root->token.length > 0, "Malformed argument data." );
	assert( root->child != NULL, "Malformed argument data." );
	assert( root->sibling == NULL, "Not supporing multiple files." );
	assert( root->kind == file_node, "Malformed argument data." );
	struct ast_node* global = root->child;
	assert( global->child != NULL, "Malformed argument data." );
	assert( global->sibling == NULL, "Not supporing multiple globals." );
	assert( global->kind == procedure_node, "Malformed argument data." );
	validate_start_procedure( global );
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
	string_added = sprintf( &output.literal_string.data[ output.literal_string.length ], "%d", root->token.length - 2 );
	output.literal_string.length += string_added;
	string_append( &output.literal_string, " x i8 ] c", 9 );
	string_append( &output.literal_string, root->token.raw, root->token.length );
	output.literal_string_number += 1;
	return output.literal_string_number - 1;
}

void output_procedure_call( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == procedure_call_node, "Malformed argument." );
	string_append( &output.file, "call ", 5 );
	if( char_array_equal( root->token.raw, "write_syscall", 13 )){
		string_append( &output.file, "i64 @write_syscall( i64 ", 24 );
		string_append( &output.file, root->sibling->sibling->token.raw, root->sibling->sibling->token.length );
		string_append( &output.file, ", ptr @.literal_string.", 23 );
		i32 index = output_add_string( root->sibling->child );
		string_alloc( &output.file, 32 );
		i32 string_added = sprintf( &output.file.data[ output.file.length ], "%d", index );
		output.file.length += string_added;
		string_append( &output.file, ", i64 ", 6 );
		string_alloc( &output.file, 32 );
		string_added = sprintf( &output.file.data[ output.file.length ], "%d", root->sibling->child->token.length - 2 );
		output.file.length += string_added;
		string_append( &output.file, " )", 2 );
	} else {
		assert( false, "Not supporting arbitrary procedure calls yet." );
	}
}

void output_register( struct ast_node* root ){
	assert( root != NULL, "Malformed argument." );
	assert( root->kind == register_node, "Malformed argument." );
	string_append( &output.file, "\t%", 2 );
	string_append( &output.file, root->token.raw, root->token.length );
	string_append( &output.file, " = ", 3 );
	struct ast_node* value = root->child->sibling;
	if( value->kind == register_node ){
		string_append( &output.file, "add ", 4 );
		string_append( &output.file, root->child->token.raw, root->child->token.length );
		string_append( &output.file, " %", 2 );
		string_append( &output.file, value->token.raw, value->token.length );
		string_append( &output.file, ", 0", 3 );
	} else if ( value->kind == literal_number_node ){
		string_append( &output.file, "add ", 4 );
		string_append( &output.file, root->child->token.raw, root->child->token.length );
		string_append( &output.file, " ", 1 );
		string_append( &output.file, value->token.raw, value->token.length );
		string_append( &output.file, ", 0", 3 );
	} else if ( value->kind == addition_node ){
		string_append( &output.file, "add ", 4 );
		string_append( &output.file, root->child->token.raw, root->child->token.length );
		string_append( &output.file, " ", 1 );
		if( value->sibling->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, value->sibling->token.raw, value->sibling->token.length );
		} else if ( value->sibling->kind == literal_number_node ){
			string_append( &output.file, value->sibling->token.raw, value->sibling->token.length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
		string_append( &output.file, ", ", 2 );
		if( value->child->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, value->child->token.raw, value->child->token.length );
		} else if ( value->child->kind == literal_number_node ){
			string_append( &output.file, value->child->token.raw, value->child->token.length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
	} else if ( value->kind == procedure_call_node ){
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
	assert( char_array_equal( root->token.raw, "return", 6 ), "Bad root not jump return for now." );
	if( root->child->kind == addition_node ){
		string_append( &output.file, "\t%", 2 );
		string_append( &output.file, root->token.raw, root->token.length );
		string_append( &output.file, ".1 = add ", 9 );
		string_append( &output.file, procedure->child->child->token.raw, procedure->child->child->token.length );
		string_append( &output.file, " ", 1 );
		if( root->child->sibling->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, root->child->sibling->token.raw, root->child->sibling->token.length );
		} else if ( root->child->sibling->kind == literal_number_node ){
			string_append( &output.file, root->child->sibling->token.raw, root->child->sibling->token.length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
		string_append( &output.file, ", ", 2 );
		if( root->child->child->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, root->child->child->token.raw, root->child->child->token.length );
		} else if ( root->child->child->kind == literal_number_node ){
			string_append( &output.file, root->child->child->token.raw, root->child->child->token.length );
		} else {
			assert( false, "Bad addition root not valid for now." );
		}
		string_append( &output.file, "\n\tret ", 6 );
		string_append( &output.file, procedure->child->child->token.raw, procedure->child->child->token.length );
		string_append( &output.file, " %", 2 );
		string_append( &output.file, root->token.raw, root->token.length );
		string_append( &output.file, ".1", 2 );
	} else if( root->child->kind == member_node ){
		assert( char_array_equal( root->child->sibling->token.raw, "argument", 8 ), "Only supporting argument.count member." );
		assert( char_array_equal( root->child->child->token.raw, "count", 5 ), "Only supporting argument.count member." );
		string_append( &output.file, "\tret ", 5 );
		string_append( &output.file, procedure->child->child->token.raw, procedure->child->child->token.length );
		string_append( &output.file, " %argument.count", 16 );
	} else {
		string_append( &output.file, "\tret ", 5 );
		string_append( &output.file, procedure->child->child->token.raw, procedure->child->child->token.length );
		string_append( &output.file, " ", 1 );
		if( root->child->kind == register_node ){
			string_append( &output.file, "%", 1 );
			string_append( &output.file, root->child->token.raw, root->child->token.length );
		} else if ( root->child->kind == literal_number_node ){
			string_append( &output.file, root->child->token.raw, root->child->token.length );
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
	string_append( &output.file, root->child->child->token.raw, root->child->child->token.length );
	string_append( &output.file, " @", 2 );
	string_append( &output.file, root->token.raw, root->token.length );
	string_append( &output.file, "( ", 2 );
	assert( *root->child->child->sibling->child->token.raw == '@', "Bad procedure argument for now." );
	assert( *root->child->child->sibling->child->child->token.raw == '@', "Bad procedure argument for now." );
	string_append( &output.file, "ptr %", 5 );
	string_append( &output.file, root->child->child->sibling->token.raw, root->child->child->sibling->token.length );
	string_append( &output.file, ".data, ", 7 );
	string_append( &output.file, "i64 %", 5 );
	string_append( &output.file, root->child->child->sibling->token.raw, root->child->child->sibling->token.length );
	string_append( &output.file, ".count ){\n", 10 );
	struct ast_node* statement = root->child->child->sibling->sibling;
	while( 1 ){
		if( statement->kind == register_node ){
			output_register( statement );
		} else if( statement->kind == jump_node ){
			output_jump( statement, root );
			break;
		} else {
			print_ast();
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
	root->token.raw[ root->token.length - 2 ] = 'l';
	bool error = string_to_file( &output.file, root->token.raw );
	compiler_assert( !error, root->token, error_level, "Unable to write to file '%s'.", root->token.raw );
}

i32 main( i32 argc, char* argv[] ){
	if( argc != 2 ){
		fprintf( stderr, "Usage: ssalc file_name.sl\n" );
		exit( 1 );
	}
	root_node.token.raw = argv[ 1 ],
	root_node.token.length = strlen( argv[ 1 ]),
	root_node.token.file = argv[ 1 ],
	root_node.token.file_length = strlen( argv[ 1 ]),
	root_node.kind = file_node,
	parse_file( &root_node );
	validate_globals( &root_node );
	output_llvm( &root_node );
#ifdef DEBUG
	print_ast();
#endif
	return 0;
}

