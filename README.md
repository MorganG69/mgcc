# mgcc
- C compiler for 32 bit ARM architectures and a custom 8 bit architecture.

- Following the ANSI C standard outlined in K&R Second Edition.

Currently Implementing:
- Lexer
    - Adding features as the parser grows, generates most tokens so far.

- Parser
    - Any expression.
    - Can parse any declaration and represent it in a text format. (int x produces the string "declare x as an integer").
    
- Optimizer
    - Nothing implemented yet.

- Assembly code generation
    - Can generate constant expressions.
