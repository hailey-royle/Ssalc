
## Anatomy of a 'Hello World!'

```ssal
#include[ "basic.sl" ];

start procedure[ argumnet @@u8 : u8 ]
{
	write_count s64 = write[ standard_out, "Hello World!\n" ];
	!return[ 0 ];
}
```

The program is compiled with `ssal file_name.sl`

All programs must include a `start` procedure, this is where the program will begin execution.  
The `start` procedure must have the signature `start procedure[ argument @@u8 : u8 ]`.  

- `start` is the name of the procedure.
- `procedure` is the type of `start. all identifier declarations must hava a type.
- `[]` a procedure must have argument brackets, even if it has no arguments.
- `argument` is a procedure argument of type `@@u8` (an array of arrays of unsigned eight bit integers) which hold the command line arguments.
- `:` separates the argumnets from the return values, in this case, there is one of each, it must be included even if there are no return values.
- `u8` is the type of the return value, the error code given back when this program exits.

Every procedure must have a body, which is wrapped in `{}`.  
The `start` procedure has only one block, with two statments.  
All statments must end in a `;`.  

The first statment is `write_count s64 = write[ standard_out, "Hello World!\n" ];`. 
This describes a call to the procedure `write`, which is defined as `write procedure[ destination file, string @u8 : s64 ]`.  
`standard_out` is the file that will be written to.  
`"Hello World!\n"` is a litteral string, which will be written to the file.  
`write_count` is the number of bytes written, all return values must be assigned even if they are not used.  

The second statment is `!return[ 0 ]`. 
All routines must end with a jump to the next block to be executed.  
`return` jumps to the procedure's call site.  
This exits the program, becuase `return` is returning from the `start` procedure.  

This program uses the `write` procedure and the `standard_out` file, which is defined in the standard library "core.sl".  
Using a library procedure requires that `#include[ "library_file_name" ];` be somewhere in the program.  
The `#` prefix on a procedure call tells the compiler to run the procedure when compiling the program.  
`#include procedure[ name []u8 :]` is a compiler defined procedure that will compile library code with user code.  
The `#` prefix on a procedure tells the compiler thatt the procedure must be run when compiling the program.  

