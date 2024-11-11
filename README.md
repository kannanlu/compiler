# compiler
a collection modern compiler related codes

## kaleidoscope
An implementation of LLVM-based compiler "Kaleidoscope", see the tutorial https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html .This is a simple language with only one data type 64-bit float, if/then/else construct, for loop, user defined operators, JIT compilation and a simple command line interface. 

### lexer
The lexer returns several special cases (EOF, def, extern, identifier, number) as special tokens, and others as ASCII numbers. 

