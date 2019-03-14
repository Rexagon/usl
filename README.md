## USL - useless scripting language

### Syntax definition
```
program = general_statement+

general_statement = function_declaration | statement

statement = while_loop |
            for_loop |
            branch |
            (variable_declaration SEMICOLON) |
            (expression SEMICOLON)

function_declaration = KW_FUNCTION PAR_O function_arguments PAR_C BR_O function_statement* BR_C

function_statement = statement | (KW_RETURN [expression] SEMICOLON)

function_arguments = [ID (COMMA ID)*]

while_loop = KW_WHILE PAR_O expression PAR_C BR_O loop_body BR_C

for_loop = KW_FOR PAR_O for_condition PAR_C BR_O loop_statement* BR_C

for_condition = [variable_declaration] SEMICOLON [expression] SEMICOLON [expression]

loop_statement = statement | (KW_BREAK SEMICOLON) | (KW_CONTINUE SEMICOLON)

branch = KW_IF PAR_O expression PAR_C BR_O statement* BR_C [else_branch]

else_beanch = KW_ELSE BR_O statement* BR_C

variable_declaration = KW_LET ID [OP_ASSIGN expression]

expression = logical_or_expression |
             (unary_expression OP_ASSIGN expression)

logical_or_expression = logical_and_expression |
                        (logical_or_expression OP_OR logical_and_expression)

logical_and_expression = equality_expression |
                         (logical_and_expression OP_AND equality_expression)

equality_expression = relational_expression |
                      (equality_expression OP_EQ relational_expression) |
                      (equality_expression OP_NEQ relational_expression)

relational_expression = additive_expression |
                        (relational_expression OP_LT additive_expression) |
                        (relational_expression OP_LEQ additive_expression) |
                        (relational_expression OP_GT additive_expression) |
                        (relational_expression OP_GEQ additive_expression)

additive_expression = multiplicative_expression |
                      (additive_expression OP_ADD multiplicative_expression) |
                      (additive_expression OP_MINUS multiplicative_expression)

multiplicative_expression = unary_expression |
                            (multiplicative_expression OP_MUL unary_expression) |
                            (multiplicative_expression OP_DIV unary_expression)

unary_expression = postfix_expression |
                   (OP_INC unary_expression) |
                   (OP_DEC unary_expression) |
                   (OP_PLUS unary_expression) |
                   (OP_MINUS unary_expression) |
                   (OP_NEG unary_expression)

postfix_expression = primary_expression |
                     (postfix_expression OP_INC) |
                     (postfix_expression OP_DEC) |
                     (postfix_expression STRUCT_REF ID) |
                     (postfix_expression PAR_O call_arguments PAR_C)

primary_expression = ID | NUMBER | STRING | (PAR_O expression PAR_C)

call_arguments = [expression (COMMA expression)*]

KW_LET = "let"
KW_IF = "if"
KW_ELSE = "else"
KW_WHILE = "while"
KW_FOR = "for"
KW_BREAK = "break"
KW_CONTINUE = "continue"
KW_FUNCTION = "function"
KW_RETURN = "return"

ID = /[a-zA-Z_]/
STRING = /"(\\.|[^"])*"/
NUMBER = /-?[0-9]+\.?[0-9]*/

OP_ASSIGN = "="
OP_OR = "||"
OP_AND = "&&"
OP_EQ = "=="
OP_NEQ = "!="
OP_LT = "<"
OP_LEQ = "<="
OP_GT = ">"
OP_GEQ = ">="
OP_PLUS = "+"
OP_MINUS = "-"
OP_MUL = "*"
OP_DIV = "/"
OP_INC = "++"
OP_DEC = "--"
OP_NEG = "!"

STRUCT_REF = "."

PAR_O = "("
PAR_C = ")"
BR_O = "{"
BR_C = "}"
BRACKET_O = "[" #not used
BRACKET_C = "]" #not used

COMMA = ","
SEMICOLON = ";"
```

### Generating project
Ensure latest versions of CMake and C++ compiler are installed.
```
mkdir build && cd build
cmake ..

# On Windows use
cmake --build . --config Release

# On Linux use
make
```
