
# Ssal Language Refrence

## Introduction

Ssal is a systems programming language designed to closely map how code is written to how it will be executed by the computer.  
It is imperative, procedural, single static assigned, staticly typed, and manual memory managed.  

## Whitespace

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

## Literals

### Decimal Literal

`0, 1, 2, 3, 4, 5, 6, 7, 8, 9, _` - digits.
A decimal literal is a sequence of digits.
It can be assigned to any integer or float that it does not overflow.
A `_` does not change the value, and can not be the first digit.


### Hexidecimal Literal

`0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f, _` - digits.
A hexidecimal literal is a sequence of digits.
It can be assigned to any integer or float that it does not overflow.
If it is assigned to a float, it will represent the value as bytes.
An `_` does not change the value, and can not be the first digit.
A `0x` prefix must be included.


### Floating Point Literal

`0, 1, 2, 3, 4, 5, 6, 7, 8, 9, _` - digits.
A hexidecimal literal is two sequences of digits separated by `.`.
It can be assigned to any float that it does not overflow.
It can be assigned to an integer if doing so would not truncate the value.
An `_` does not change the value, and can not be the first digit.


### String Literal

`"` - string start / end.
A string literal is the bytes between two `"`.
It is encode in UTF-8.
The `\` chanracter escapes a following sequence of characters.
Valid sequences are as follows.
- `\x` - followed by two hexidecimal characters to form a eight bit literal.
- `\0` - null, equivalent to \x00
- `\b` - bakcspace, equivalent to \x08
- `\t` - horizontal tab, equivalent to \x09
- `\n` - newline, equivalent to \x0a
- `\v` - vertical tab, equivalent to \x0b
- `\f` - form feed, equivalent to \x0c
- `\r` - carrage return, equivalent to \x0d
- `\e` - escape, equivalent to \x1b
- `\"` - double quote, equivalent to \x22
- `\\` - backslash, equivalent to \x5c
- `\d` - delete, equivalent to \x7f


### Code Literal

`'` - code start / end.
A code literal is the bytes between two `'`.
A `'` in a string literal does not end the code literal.
A code literal must be a valid sequence of tokens.


## Identifier

Identifiers are anything in the program defined by the program, registers, types, procedures etc.
They are made of combinations of ascii letters, numbers and '_'.
The first character must not be an number.


## Operators and Punctuation

`\{` - comment open  
`}\` - comment close  
`[`  - argument open  
`]`  - argument close  
`(`  - expression open  
`)`  - expression close  
`{`  - scope open  
`}`  - scope close  
`,`  - list seperator  
`;`  - statement end  
`:`  - result  
`!`  - jump call  
`=`  - assignment  
`#`  - execute at compilation  
`"`  - string open / close  
`'`  - code open / close  
`.`  - composite member operator  
`^`  - pointer type / refrence operator / derefrence operator  
`@`  - array type / index operator  
`?`  - conditional  
`$`  - selection  
`+`  - addition operator  
`-`  - subtraction / negation operator  
`*`  - multiplication operator  
`/`  - divistion operator  
`%`  - modulo operator  
`<<` - shift left operator  
`>>` - shift right operator  
`~`  - bitwise not operator  
`&`  - bitwise and operator  
`|`  - bitwise or operator  
`\`` - bitwise xor operator  
`~~` - logical not operator  
`&&` - logical and operator  
`||` - logical or operator  
`==` - equals operator  
`<`  - less then operator  
`>`  - greator then operator  
`<=` - less then or equals operator  
`>=` - greator then or equals operator  
`<>` - less or greator then operator  


## Procedure

Procedures are a list of routines.
They are the hightest level of program control flow.

Procedures are defined as:  
`name procedure[ return_values : arguments ]{ ... }`

```ssal
#inlcude[ "core.sl" ];

start procedure[ i64 : argument @@i8 ] \{1}\
{
	num i64 = do_stuff[ 1 ]; \{2}\
	!return[ 0 ]; \{3}\
}

\{ 1 - The start procedure.
	This is the entry point in every Ssal program.
	It must have one return value of type `i64`, which is the program exit code, and one argument of type '@@i8', which are the command line arguments.
}\ 

\{ 2 - A procedure call.
	This calls the procedure `do_stuff` with the one argument `1`
}\ 

\{ 3 - Procedure return.
	When the execution of a procedure hits `!return` program execution returns to the calling procedure.
	This is in the `start` proedure, so it exits the program with an exit code, in this case `0`.
}\ 

do_stuff procedure[ i64 : arg i64 ]
{
	!return[ arg + 2 * 3 ];
}
```


## Routine

Routines are blocks of statements.
Routines must be contained a procedure.

Routines are defined as:  
`name routine[ arguments ]; body`

```ssal
#inlcude[ "core.sl" ];

start procedure[ i64 : argument @@i8 ] \{1}\
{
	? input < 0 : !start[ argument ], !routine_1[]; \{2}\

routine_1 routine[]; \{3}\
	num i64 = do_stuff[ arugment.count ];
	!return[ num ]; \{4}\
}

\{ 1 - A procedure implicitly contains a routine. }\ 

\{ 2 - Routine jump
	At the end of every routine, there is a jump to the next routine.
	This jump is contitional, it can be read as "If input < 0 then jump start[ argument ] else jump routine_1[]".
	Every procedure can be restarted with a jump to `start`.
	A routine call can have arguments, and if it does they must be passed between the '[]';
}\

\{ 3 - Routine definition }\

\{ 4 - Return jump
	A return is a routine call that exits the procedure.
}\

do_stuff procedure[ i64 : arg i64 ]
{
	!return[ arg + 2 * 3 ];
}
```

## Statement

### Register

Registers are a name for a value.
`identifier: type = expression;` is the basic form for a register assignment statement.
A registers value can not be changed once it hase been assigned, unless the assignment code has been rerun.


### Expression

Expressions are built with any value: literal, register, procedure return.
Precidence is explicitly defined by wrapping in `()`.


## Condition


```ssal
\{
basic contidional format:
? expression : if_true, if_false
}\

reg s64 = ? a == b : 0, b;
reg2 s64 = something * ? a == b : 0, b + something_else;
\{ reg2 s64 = something * ( ? a == b : 0, b ) + something_else; }\
? reg > 20 : proca[], procb[];  \{ calls must not return anything }\
ret s64 = ? reg > 20 : procc[], procd[];  \{ calls must return the same type }\
? ret == reg : !jumpa[], !jumpb[];
```


## Match

```ssal
\{
basic match format:
$ expression : default,
	a -> a_handle,
	b -> b_handle,
	c -> c_handle;
}\

\{ has the same usage rules as a condition }\
```

## Types

Basic types:

| name | kind | size |
|---|---|---|
| i8 | singed integer | 8 |
| i16 | singed integer | 16 |
| i32 | singed integer | 32 |
| f32 | floating point | 32 |
| i64 | singed integer | 64 |
| f64 | floating point | 64 |

Other types:

`^` - Pointer to type of the next token (`^i64`), or omitted for a raw pointer.

`@` - Array of type of the next token, or omitted for a array with a raw pointer. 
Arrays are build in to the language, there are two types, static and dynamic.
The size of a static array can not be changed.
The size of a dynamic array can be changed, given it was allocated by an allocator allowing resizing.

Array internal definition:

```ssal
array structure[
	data ^\{type}\,
	count s64
	allocated s64,
];
```


### Structure

Structures are a combination of several fields.

```ssal
vector3 structure[
	x f64,
	y f64,
	z f64
];

vector3_instance vector3 = [ 0, 5.5, 893.333 ];
```

### Union

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
	parts parts,
];
```


### Enumeration

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
### Any

## Metaprogramming
## Assembly
## core.sl
### Allocators
### Format Strings

