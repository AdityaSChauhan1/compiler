## Project Overview

The project consists of several components:

•	Lexical Analysis: Implemented using a lexer that scans the input code, identifies tokens (keywords, identifiers, operators, etc.), and generates a token stream.

•	Parsing: A recursive descent parser processes the token stream, validating the syntax based on defined grammar rules and constructing the parse tree.

•	Semantic Analysis: This phase checks for semantic errors, such as undeclared variables, type mismatches, and missing return statements. It maintains a symbol table to track declared variables and their scopes.

•	Intermediate Code Generation: The intermediate code generator translates the parsed statements into a low-level, three-address code representation, handling constructs like assignments, conditional statements, and loops.

•	Data flow between phases is achieved through token streams and intermediate data structures like symbol tables and parse trees.

•	Intermediate code is written to a file (ir.txt), serving as the output for further processing or optimization.

## Features


Lexical analysis of C code 

Syntex analysis of C code

Semantic analysis of C code


Intermediate code generation of C code

Target Code Generation of C code

Parse Tree generation

Support for:

airthmetic operations with precedence 

for loops 

if else loops nested

While loops 

variable decleration



## Command Line Usage

You can use the command line interface to generate AST and IR for Python code:

```bash
  gcc main.c lexer.c parser.c semantic.c ir.c -o compiler
  ./compiler input.txt
  dot -Tpng tree.dot -o tree.png
  open tree.png
```

This will display both the Parse tree and the intermediate code for the C language




## Future Enhancements

Potential future enhancements include:

More comprehensive C language support

Addition of functions and arrays 

Additional visualizations and interactive elements

Support for more complex C features (Dynamic memory 
allocation, recurssion, etc.)




## Contribution

Contributions are welcome! Feel free to submit pull requests or open issues for bugs or feature requests.
