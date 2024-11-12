# compiler

a collection modern compiler related codes

## kaleidoscope

An implementation of LLVM-based compiler "Kaleidoscope", see the tutorial https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html .This is a simple language with only one data type 64-bit float, if/then/else construct, for loop, user defined operators, JIT compilation and a simple command line interface.

### lexer

The lexer returns several special cases (EOF, def, extern, identifier, number) as special tokens, and others as ASCII numbers.

### AST

The abstract syntax tree (AST) are node classes that define basic expression types, include number, variable, binary operation, function call, function prototype, and function. Notice that the only data type in this language is float-64, so there is no type fields in the function.
