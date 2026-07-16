
## Language Philosophy

A programming language is a meduim to control a computer.

#### A programming language should be written in a way that cleanly maps to how the hardware works.

- The programer should be able to have a good idea of how unoptomized machine code will run.
- The programer should be able to have a reasonable idea of how optomized machine code might run.


#### A programming language should be able to express anything the computer can do.

- The language should be easily interoperable with assembly.
- The language should not nessessarily make it easy to express something the computer can do.


#### A programming language should provide a small number of simple constructs.

- These construct's functionality should not significatly overlap.
- These constructs should not have arbitrary restrictions.


#### A programming language should be able to create programs without any run-time or compile-time support.

- The langauge should not require an operating system.
- The langauge should not require any standard library components.
- The language should not de jour or de facto require an outside build system.


#### A programming language should prodispose the programmer to writing 'good' code.

- A programming language should predispose the programmer to writing easily understandable code.
- A programming language should predispose the programmer to writing high preformance code.
- A programming language should predispose the programmer to writing expressive code.


#### A programming language should be relatively easy to make a compiler for.

- A programming language should have a well defined specification.


## Influences

In order of significance, which other languages influenced Ssal.


**LLVMIR**

To my understanding, the single static assignment paradigm was invented only for the narrow puropse of compiler optomizations.
But why is ssa so good at turning structured language code into preformant assembly?
I think its becuase ssa mirrors what is actualy happening on modern cpus.
Optimizing out memory access reflects how slow memory is.
Seperating code by blocks with jumps reflects how to cpu will attempt to do many operations in parrallel, and how this can be inturputed by a branch.
the register primative ( as opposed to memeory ) reflects the larger number of physical registers and the arbitraity of assembly registers.

Ssal began as an attempt to clean llvmir syntax.
llvmir is ment to be a compiler backend language, but it is too verbose for everyday programming.
The basic structure of llvm, pirticulary its register primative and blocks, are good foundations to build on.
Concepts exclusivly ment to facilitate optomization are removed.
Some language features are made more human readable.
For example, geps are removed for array indexing / structure members, and phis are removed for routine arguments.


**C**

Ssal is ment to be a modern c.
Not in the sence that it is c with modern sensibilites, or with its weaknesses strengthened,
 but a reminagination of what C could have been if it was designed around current computer architecture.

A language should have a limited set of constructs which are supplemented by a standard library.
Computers have undefined behavior and are not memory safe, a langauge does not need to protect the programmer from this fact.


**Jai**

Metaporgamming with arbitrary compile time execution is much preferable to text replacement macros.
Memory allocation should be done by different allocators with different priorities.
Programs should not need any external programs to build itself.

