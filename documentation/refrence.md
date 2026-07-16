
# Ssal Language Refrence

## Introduction

Ssal is a systems programming language designed to closely map how code is written to how it will be executed by the computer.  
It is imperative, procedural, single static assigned, staticly typed, and manual memory managed.  

Whitespace is used only to seperate tokens. The following are all equivalent programms.

```ssal
start procedure[ arguments @@u8 : u8 ]
{
	!return[ 0 ];
}
```

```ssal
start procedure[arguments@@u8:u8]{!return[0];}
```

```ssal
     start     procedure
  [arguments    @@u8:u8

       ]           {!
        return[0];}
```


## Comments

```ssal

start procedure[ arguments @@u8 : u8 ]
{

\{ This is a comment. }\
\{ There are no single line comments \{ Comments can be nested. }\ }\

\{	!return[ 64 ]; }\
	!return[ 0 ];
}

```


## Procedure

```ssal
#include[ "core" ];

start procedure[ arguments @@u8 : u8 ]  \{1}\
{
	do_nothing[];  \{4}\
	x s64 = square[ 8 ];  \{5}\
	!loop_arguments[ 0 ];

loop_arguments routine[ index u64 ];
	bytes s64 = write_format[ standard_out, "index: \% argument: \"\%\"\n"  [ index, arguments[ index ]]];  \{6}\
	? index < arguments.count : !loop_arguments[ index + 1 ], !return[ 0 ];

\{ procedure signature: write_format procedure[ destination file, format @u8, argument @any : s64 ] }\

}

do_nothing procedure[:]  \{2}\
{
	!return[];
}

square procedure[ x s64 : s64 ] \{3}\
{
	!return[ x * x ];
}
```


\{1}\ The `start` procedure is the program entry point, it must be in every program.
It must have one argument of type `@@u8`, which are the command line arguments.
It is convention to name it `aruguments`, but it can be named anything.
`start` returns an exit code of type `u8`.

\{2}\ The most simple procedure, it has no arguments, does nothing, and returns nothing.

\{3}\ A basic procedure that returns the square of the argument.

\{4}\ A procedure call to with no arguments and no return value.

\{5}\ A procedure call with one argument and one return value.

\{6}\ A procedure call with three arguments, a file, a format string, and a array of type `any`.
`write_format` follows the format string convention.
In the string literal "index:\% argument:\%\n", there are two format specifiers `\%`.
`write_format` will read the type data of the elements of the any array, and insert them into the format string.
The number of format specifiers must be equal to the number of formatting arguments.
If it was called as: `bytes s64 = write_format[ standard_out, "index: \% argument: \"\%\"\n", [ 2, ":)" ];`
Then it would output "index: 2 argument: ":)"" followed by a newline.


## Routine

```ssal
#include[ "core.sl" ];

start procedure[ argument @@u8 : u8 ]
{
	assert[ argument.count == 2, "Usage: $ space_to_tab filename." ];
	file_path @u8 = argument[ 1 ];
	file_string @u8 = string_from_file[ file_path ];
	assert[ file_string.data != 0, "Error reading file \%\n", file_path ];
	start_index s64 == string_seek_next_substring( file_string, 0, "        " ];
	? start_index == file_string.count : !cleanup[], !loop_file[ start_index ];  \{3}\

loop_file routine[ index s64 ];  \{1}\
	string_delete[ file_string, index, 8 ];
	string_insert[ file_string, index, "\t" ];
	new_index s64 = string_seek_next_substring( file_string, index, "        " ];
	? new_index == file_string.count : !cleanup[], !loop_file[ new_index ]; \{4}\

cleanup routine[];  \{2}\
	bytes_written s64 = string_to_file[ file_string, argument@[ 1 ]];
	assert[ bytes_written == file_string.count, "Error writing file \%\n", argument@[ 1 ]];
	!return[ 0 ];  \{4}\
}
```
\{1}\ - This is a basic routine declaration with one argument.
Routine arguments get reset every routine call.

\{2}\ - This is a routine with no arguments.

\{3}\ - This is a routine jump, in this case a conditional jump.
Conditional jumps have the format `? expression : !expression_true[], !expression_false[];`
If it jumps to `loop_file`, it passes an argument, which becomes the value of `index`.

\{4}\ - If `new_index <> file_string.count` then `loop_file` will be jumped to again.
The value of register `index` will be reset to `new_index`, and the procedure code will run again.


\{4}\ - `return` is a jump, but not a routine call.
It exits the procedure with its return arguments.

## Register

TODO

## Types

Basic types:

| name | kind | size |
|---|---|---|
| u8 | unsinged integer | 8 |
| s8 | singed integer | 8 |
| u16 | unsinged integer | 16 |
| s16 | singed integer | 16 |
| u32 | unsinged integer | 32 |
| s32 | singed integer | 32 |
| f32 | floating point | 32 |
| u64 | unsinged integer | 64 |
| s64 | singed integer | 64 |
| f64 | floating point | 64 |

Other types:

`^` - Pointer to type of the next token (`^u32`), or omitted for a raw pointer.

`@` - Array of type of the next token, or omitted for a array with a raw pointer. 
Arrays are build in to the language, there are two types, static and dynamic.
The size of a static array can not be changed.
The size of a dynamic array can be changed, given it was allocated by an allocator allowing resizing.

Static array:

```ssal
static_array structure[
	data ^\{type}\,
	count s64
];

dynamic_array structure[
	data ^\{type}\,
	count s64,
	allocated s64,
	allocator allocator_procedure
];
```

Structures are a combination of several fields.

```ssal
vector3 structure[
	x f64,
	y f64,
	z f64
];

vector3_instance vector3 = [ 0, 5.5, 893.333 ];
```

Unions are a combination of several types into the same memory.

```ssal
parts structure[
	r u8,
	g u8,
	b u8,
	a u8,
];

color union[
	full u32,
	part parts,
];
```

Enumerations can only be assigned to one of their members.

```ssal
token enumeration[
	error_token,
	procedure_token,
	routine_token,
	structure_token,
	union_token,
	enumeration_token
];
```

## Metaprogramming
## Assembly
## core.sl

