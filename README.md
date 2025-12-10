# Mini LL(1) Compiler (Lexer + Parser)

A single C program that tokenizes and parses a small teaching language using a hand-built DFA lexer and an LL(1) parsing table.

## Files
- final.c: combined lexer, parser, and main driver.
- check.c: sample input program consumed by the lexer/parser (edit this to test your own code).

## Build
- gcc final.c -o compiler

## Run
- ./compiler    # on Windows: compiler.exe
- The program reads `check.c`, prints the lexical tokens, then shows the LL(1) parse trace and a final verdict.

## Language snapshot
- First line must be `#include<stdio.h>`.
- Types: int, dec. Functions include `main` or names ending in `Fn`.
- Variables: `Type id = Expr ..` where `..` terminates a statement.
- Print: `printf(id)..`. Return and break: `return value..`, `break..`.
- Loop label + while form: `loop_name: while(Type id < number..) { ... }` with the `..` closing the condition.
- Expr supports identifiers, numbers, function calls `IdFn(id)`, and `+` chains.
- Single-line `//` comments are skipped; unexpected chars abort lexical analysis.

## Customizing input
- Replace the contents of check.c with your program, or change the filename in main�s call to `lex_file("check.c", ...)`.

## Notes
- Statement terminator is `..` (not `;`).
- Only the tokens in the DFA are recognized; features like strings, minus, or multi-argument calls are not supported.