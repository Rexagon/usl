## USL - useless scripting language

## Syntax
```
program = statement+

statement = 
    'if' paren_expr statement |
    'if' paren_expr statement 'else' statement |
    'while' paren_expr statement |
    '{' statement* '}' |
    expr ';' |
    ';'

paren_expr = '(' expr ')'

expr = UN_OP term (UN_OP term)*

term = factor (BN_OP factor)*

factor = var | NUMBER | paren_expr

var = [a-zA-Z_]+

string = '"' [a-zA-Z_]+ '

NUMBER = '0' | ([1-9][0-9]*)

UN_OP = [+-]
BN_OP = [*/]

SPACE = [ \n\r\t]
```