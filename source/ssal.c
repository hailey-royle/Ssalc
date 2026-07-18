#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "assert.h"
#include "string.h"
#include "tui.h"
#include "array.h"

enum ast_node_kind {
	error_node = 0,
	identifier_node,
	literal_integer_node,
	procedure_node,
	argument_open_node,
	argument_close_node,
	scope_open_node,
	scope_close_node,
	statment_end_node,
	result_node,
	array_node,
	jump_node,
};

struct ast_node {
	struct ast_node* child;
	struct ast_node* sibling;
	char* raw;
	uint32_t raw_length;
	uint32_t line;
	uint32_t column;
	enum ast_node_kind kind;
};

struct ast_node_array {
	struct ast_node* data;
	uint32_t count;
	uint32_t allocated;
};
#define ast_node_array_new( name ) array_new( (struct array*) name, sizeof( struct ast_node ));

struct source_file {
	struct ast_node_array raw_node;
	struct string source;
	struct string output;
	struct ast_node* root_node;
	char* name;
	uint32_t index;
	uint32_t line;
	uint32_t column;
};

enum compiler_error_level {
	compiler_level,
	error_level,
	warning_level,
	note_level,
};

int line_display_length( char* line, int bytes ){
	int length = 0;
	for( ; bytes > 0 && *line != '\n' && *line != '\r' && *line != '\0'; line++ ){
		if( *line == '\t' ){
			length += 8;
		} else if( *line & 0x80 && ~*line & 0x40 ){
			/* non starting utf8 */
		} else {
			length += 1;
		}
		bytes -= 1;
	}
	return length;
}

int line_length( char* line ){
	int i = 0;
	for( ; *line != '\n' && *line != '\r' && *line != '\0'; line += 1 ){
		i += 1;
	}
	return i;
}

void report_error( struct source_file* source, struct ast_node* node, enum compiler_error_level level, char* message ){
	char* start = node->raw - node->column + 1;
	int length = line_display_length( start, node->column );
	int bytes = line_length( start );
	if( level == compiler_level ){
		printf( "%s%sCompiler%s ", ansi_bold_start, ansi_foreground_magenta, ansi_foreground_default );
	} else if( level == error_level){
		printf( "%s%sError%s ", ansi_bold_start, ansi_foreground_red, ansi_foreground_default );
	} else if( level == warning_level ){
		printf( "%s%sWarning%s ", ansi_bold_start, ansi_foreground_yellow, ansi_foreground_default );
	} else if( level == note_level ){
		printf( "%s%sNote%s ", ansi_bold_start, ansi_foreground_green, ansi_foreground_default );
	}
	printf( "%s:%d:%d%s %s\n%.*s\n%*c\n", source->name, node->line, node->column, ansi_bold_end, message, bytes, start, length, '^' );
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

bool char_array_equal( char* a, char* b, int n ){
	for( int i = 0; i < n; i++ ){
		if( a[ i ] == 0 || b[ i ] == 0 || a[ i ] != b[ i ] ){
			return false;
		}
	}
	return true;
}

struct ast_node* next_node( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( file->index < file->source.len, "Source file overflow." );
	struct ast_node* node = ast_node_array_new( file );
	while( char_is_space( file->source.data[ file->index ])){
		if( '\n' == file->source.data[ file->index ]){
			file->column = 1;
			file->line += 1;
		} else {
			file->column += 1;
		}
		file->index += 1;
	}
	if( file->index >= file->source.len ){
		return node;
	}
	node->raw = &file->source.data[ file->index ];
	node->line = file->line;
	node->column = file->column;
	if( char_is_identifier_start( file->source.data[ file->index ])){
		do{
			file->column += 1;
			file->index += 1;
			node->raw_length += 1;
		} while( char_is_identifier( file->source.data[ file->index ]));
		if( char_array_equal( node->raw, "procedure", 9 )){
			node->kind = procedure_node;
		} else {
			node->kind = identifier_node;
		}
	} else if( char_is_integer_start( file->source.data[ file->index ])){
		do{
			file->column += 1;
			file->index += 1;
			node->raw_length += 1;
		} while( char_is_integer( file->source.data[ file->index ]));
		node->kind = literal_integer_node;
	} else if( '[' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = argument_open_node;
	} else if( ']' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = argument_close_node;
	} else if( '{' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = scope_open_node;
	} else if( '}' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = scope_close_node;
	} else if( ';' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = statment_end_node;
	} else if( ':' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = result_node;
	} else if( '@' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = array_node;
	} else if( '!' == file->source.data[ file->index ]){
		file->column += 1;
		file->index += 1;
		node->raw_length += 1;
		node->kind = jump_node;
	} else {
		report_error( file, node, error_level, "Invalid syntax." );
	}
	return node;
}

struct ast_node* parse_type( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	struct ast_node* local_root_node = next_node( file );
	struct ast_node* node = local_root_node;
	while( 1 ){
		if( node->kind == identifier_node ){
			break;
		} else if( node->kind == array_node ){
			node->child = next_node( file );
			node = node->child;
		} else {
			report_error( file, node, error_level, "Expected type. ('@' or '^' or type name)" );
		}
	}
	return local_root_node;
}

struct ast_node* parse_procedure_declaration_arguments( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	struct ast_node* local_root_node = next_node( file );
	if( local_root_node->kind != argument_open_node ){
		report_error( file, local_root_node, error_level, "Expected '[' after 'procedure'." );
	}
	local_root_node->sibling = next_node( file );
	struct ast_node* node = local_root_node->sibling;
	if( node->kind != identifier_node ){
		report_error( file, node, error_level, "Expected argument name." );
	}
	node->child  = parse_type( file );
	node->sibling = next_node( file );
	node = node->sibling;
	if( node->kind != result_node ){
		report_error( file, node, error_level, "Expected '->' after procedure argument list." );
	}
	node->sibling = parse_type( file );
	node = node->sibling;
	node->sibling = next_node( file );
	node = node->sibling;
	if( node->kind != argument_close_node ){
		report_error( file, node, error_level, "Expected ']' after procedure arguments." );
	}
	return local_root_node;
}

struct ast_node* parse_jump( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	struct ast_node* local_root_node = next_node( file );
	struct ast_node* node = local_root_node;
	if( node->kind != identifier_node ){
		report_error( file, node, error_level, "Expected block name identifier after jump." );
	}
	node->child = next_node( file );
	node = node->child;
	if( node->kind != argument_open_node ){
		report_error( file, node, error_level, "Expected '[' after jump name." );
	}
	node->sibling = next_node( file );
	node = node->sibling;
	if( node->kind != literal_integer_node ){
		report_error( file, node, error_level, "Expected expression after '['." );
	}
	node->sibling = next_node( file );
	node = node->sibling;
	if( node->kind != argument_close_node ){
		report_error( file, node, error_level, "Expected ']' after jump argumnets." );
	}
	node->sibling = next_node( file );
	node = node->sibling;
	if( node->kind != statment_end_node ){
		report_error( file, node, error_level, "Expected ';' at the end of a statment." );
	}
	return local_root_node;
}

struct ast_node* parse_procedure_body( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	struct ast_node* local_root_node = next_node( file );
	if( local_root_node->kind != scope_open_node ){
		report_error( file, local_root_node, error_level, "Expected '{' at the start of procedure body." );
	}
	local_root_node->sibling = next_node( file );
	if( local_root_node->sibling->kind != jump_node ){
		report_error( file, local_root_node->sibling, error_level, "Expected jump '!' as last block statment." );
	}
	local_root_node->sibling->child = parse_jump( file );
	local_root_node->sibling->sibling = next_node( file );
	if( local_root_node->sibling->sibling->kind != scope_close_node ){
		report_error( file, local_root_node->sibling->sibling, error_level, "Expected '}' at the end of a procedure body." );
	}
	return local_root_node;
}

struct ast_node* parse_global_identifier_declaration( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	struct ast_node* local_root_node = next_node( file );
	if( local_root_node->kind != procedure_node ){
		report_error( file, local_root_node->child, error_level, "All global scope identifiers (currently) must be procedures." );
	}
	local_root_node->child = parse_procedure_declaration_arguments( file );
	local_root_node->sibling = parse_procedure_body( file );
	return local_root_node;
}

void parse_file( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	file->root_node = next_node( file );
	struct ast_node* node = file->root_node;
	while( 1 ){
		if( node->kind != identifier_node ){
			report_error( file, node, error_level, "Expected identifier starting declaration when parsing global scope." );
		}
		node->child = parse_global_identifier_declaration( file );
		node->sibling = next_node( file );
		if( node->sibling->kind == error_node ){
			break;
		} else {
			report_error( file, node, error_level, "Only start procedure in global scope." );
		}
	}
}

void print_ast_node( struct ast_node* node, int depth ){
	assert( node != NULL, "Malformed argument." );
	printf( "%*c%.*s\n", depth, ' ', node->raw_length, node->raw ); 
	if( node->child != NULL ){
		print_ast_node( node->child, depth + 1 );
	}
	if( node->sibling != NULL ){
		print_ast_node( node->sibling, depth );
	}
}

void print_ast( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( file->root_node != NULL, "Malformed argument." );
	print_ast_node( file->root_node, 1 );
}

void validate_file( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( file->root_node != NULL, "Malformed argument." );
	struct ast_node* node = file->root_node;
	if( !char_array_equal( node->raw, "start", 5 )){
		report_error( file, node, error_level, "Only start procedure in global scope." );
	}
	if( !char_array_equal( node->child->raw, "procedure", 9 )){
		report_error( file, node->child->child, error_level, "'start' must be a procedure." );
	}
	if( !char_array_equal( node->child->child->sibling->raw, "argument", 8 )){
		report_error( file, node->child->child->child->sibling, error_level, "'start' must have type signature 'procedure[ argument: @@u8 -> u8 ]." );
	}
	if( !char_array_equal( node->child->sibling->sibling->child->raw, "return", 6 )){
		report_error( file, node->child->sibling->sibling->child, error_level, "Temp: 'start' procedure must '!return'." );
	}
}

void confirm_node_is_type( struct source_file* file, struct ast_node* node ){
	assert( node != NULL, "Malformed argument." );
	assert( node->raw != NULL, "Malformed argument." );
	if( node->kind == array_node ){
	} else if( char_array_equal( node->raw, "u8", 2 )){
	} else {
		report_error( file, node, compiler_level, "Unknown base type, base types:[ u8 ]." );
	}
}

struct ast_node* procedure_return_type( struct source_file* file, struct ast_node* procedure_root_node ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( procedure_root_node != NULL, "Malformed argument." );
	assert( procedure_root_node->kind == identifier_node, "Malformed argument." );
	assert( procedure_root_node->child->kind == procedure_node, "Malformed argument." );
	struct ast_node* return_type_node = procedure_root_node->child->child; // first procedure argument name
	while( return_type_node->kind != result_node ){
		return_type_node = return_type_node->sibling;
	}
	return_type_node = return_type_node->sibling;
	confirm_node_is_type( file, return_type_node );
	if( return_type_node->sibling->kind != argument_close_node ){
		report_error( file, return_type_node, compiler_level, "Temp: only supports one return value" );
	}
	return return_type_node;
}

void output_global_identifier( struct source_file* file, struct ast_node* node ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( node != NULL, "Malformed argument." );
	assert( node->kind == identifier_node, "Malformed argument." );
	string_append( &file->output, "@", 1 );
	string_append( &file->output, node->raw, node->raw_length );
}

void ouput_argument( struct source_file* file, struct ast_node* node ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( node != NULL, "Malformed argument." );
	assert( node->kind == identifier_node, "Malformed argument." );
	struct ast_node* type = node->child->child;
	confirm_node_is_type( file, type );
	if( type->kind == array_node ){
		string_append( &file->output, "ptr %", 5 );
		string_append( &file->output, node->raw, node->raw_length );
		string_append( &file->output, ".data, ", 7 );
		string_append( &file->output, "i64 %", 5 );
		string_append( &file->output, node->raw, node->raw_length );
		string_append( &file->output, ".count ", 7 );
	} else if( char_array_equal( type->raw, "u8", 2 )){
		string_append( &file->output, "i8 %", 4 );
		string_append( &file->output, node->raw, node->raw_length );
	}
	if( node->sibling->kind != result_node ){
		report_error( file, node->sibling, compiler_level, "Temp: Compiler only supports one function argument." );
	} 
}

void output_procedure_body( struct source_file* file, struct ast_node* procedure_root_node ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( procedure_root_node->kind == identifier_node, "Malformed argument." );
	struct ast_node* body_open_node = procedure_root_node->child->sibling;
	assert( body_open_node != NULL, "Malformed argument." );
	assert( body_open_node->kind == scope_open_node, "Malformed argument." );
	if( body_open_node->sibling->kind != jump_node ){
		report_error( file, body_open_node->sibling, compiler_level, "Temp: Compiler only supports jump statments." );
	}
	{
		struct ast_node* jump_call_node = body_open_node->sibling->child;
		if( !char_array_equal( jump_call_node->raw, "return", 6 )){
			report_error( file, jump_call_node, compiler_level, "Temp: Compiler only supports 'return' jump location." );
		}
		string_append( &file->output, "\tret ", 5 );
		struct ast_node* return_type_node = procedure_return_type( file, procedure_root_node );
		if( return_type_node->kind == array_node ){
			report_error( file, return_type_node, compiler_level, "Temp: Compiler only supports returning basic types." );
		} else if( char_array_equal( return_type_node->raw, "u8", 2 )){
			string_append( &file->output, "i8 ", 3 );
		}
		struct ast_node* call_argument_node = jump_call_node->child->sibling;
		if( call_argument_node->kind != literal_integer_node ){ // first argument
			report_error( file, call_argument_node, compiler_level, "Temp: Compiler only supports integer call argumnets." );
		}
		string_append( &file->output, call_argument_node->raw, call_argument_node->raw_length );
		string_append( &file->output, "\n", 1 );
	}
}

void output_procedure( struct source_file* file, struct ast_node* procedure_root_node ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( procedure_root_node != NULL, "Malformed argument." );
	assert( procedure_root_node->kind == identifier_node, "Malformed argument." );
	assert( procedure_root_node->child->kind == procedure_node, "Malformed argument." );
	assert( procedure_root_node->child->sibling->kind == scope_open_node, "Malformed argument." );
	assert( procedure_root_node->child->child->kind == argument_open_node, "Malformed argument." );
	string_append( &file->output, "define ", 7 );
	{
		struct ast_node* return_type = procedure_return_type( file, procedure_root_node );
		if( char_array_equal( return_type->raw, "u8", 2 )){
			string_append( &file->output, "i8 ", 3 );
		} else {
			report_error( file, return_type, compiler_level, "Unknown base type, base types:[ u8 ]." );
		}
	}
	output_global_identifier( file, procedure_root_node );
	string_append( &file->output, "( ", 2 );
	ouput_argument( file, procedure_root_node->child->child->sibling ); // first argument
	if( procedure_root_node->child->child->sibling->sibling->kind != result_node ){
		report_error( file, procedure_root_node->child->child->sibling->sibling, compiler_level, "Temp: Compiler only supports one function argument." );
	} 
	string_append( &file->output, "){\n", 3 );
	output_procedure_body( file, procedure_root_node );
	string_append( &file->output, "}\n", 2 );
}

void output_file( struct source_file* file ){
	assert( file != NULL, "Malformed argument." );
	assert( file->source.data != NULL, "Malformed argument." );
	assert( file->root_node != NULL, "Malformed argument." );
	char llvmir_start[] = "target triple = \"x86_64-unknown-linux-gnu\"\n\n";
	string_append( &file->output, llvmir_start, sizeof( llvmir_start ));
	output_procedure( file, file->root_node );
	if( file->root_node->sibling->kind != error_node ){
		report_error( file, file->root_node->sibling, error_level, "Temp: must have only 'start' procedure in global scope" );
	}
	return;
}

int main( int argc, char* argv[] ){
	if( argc != 2 ){
		printf( "Usage: ssalc file\n" );
		exit( 0 );
	}
	struct source_file file = { 0 };
	file.name = argv[ 1 ];
	file.line = 1;
	file.column = 1;
	bool error = string_from_file( &file.source, file.name );
	if( error ){
		printf( "Error opening file.\n" );
	}
	parse_file( &file );
	print_ast( &file );
	validate_file( &file );
	output_file( &file );
	error = string_to_file( &file.output, "out.ll" );
	if( error ){
		printf( "Error writing file.\n" );
	}
	string_free( &file.source );
	string_free( &file.output );
	free( file.raw_node.data );
	return 0;
}
