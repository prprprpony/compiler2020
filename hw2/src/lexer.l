%option noyywrap

%{

#include <stdio.h>
#include "header.h" 

int linenumber;
symtab * lookup();
symtab * ptr;
void insertID();
void printSymTab();

typedef enum TOKENS {
    // Reserved words
    RS_return, RS_typedef, RS_if, RS_else,
    RS_int, RS_float, RS_for, RS_void, RS_while,

    // Constants
    CONST_int, CONST_float, CONST_string,

    // Identifiers
    IDENT,

    // Operators
    OP_plus, OP_minus, OP_div, OP_mul,
    OP_lt, OP_gt, OP_geq, OP_leq,
    OP_neq, OP_eq, OP_or, OP_and,
    OP_not, OP_assign,

    // Separators
    SP_lbrace, SP_rbrace,
    SP_lbrack, SP_rbrack,
    SP_lparen, SP_rparen,
    SP_semicol, SP_comma, SP_dot
} TOKENS;

%}

letter  [A-Za-z]
digit   [0-9]
dot     "."
space   [ \t]+
newline "\n"
dquote  "\""
bslash  "\\"

ident   {letter}({letter}|{digit}|"_")*
iconst  {digit}+
fconst  ({digit}+{dot}{digit}*)|({dot}{digit}+)
sconst  {dquote}([^\"\\]|{bslash}.)*{dquote}

plus    "+"
minus   "-"
div     "/"
mul     "*"
lt      "<"
gt      ">"
geq     ">="
leq     "<="
neq     "!="
eq      "=="
or      "||"
and     "&&"
not     "!"
assign  "="

lparen  "("
rparen  ")"
lbrack  "["
rbrack  "]"
lbrace  "{"
rbrace  "}"
comma   ","
semicol ";"

comment \/\*([^*]|(\*+[^*\/]))*\*+\/
error   .

%%

{newline}   linenumber += 1;
{comment}   printf("%s\n", yytext);
{space}     {}

{plus}      // return OP_plus;
{minus}     // return OP_minus;
{div}       // return OP_div;
{mul}       // return OP_mul;
{lt}        // return OP_lt;
{gt}        // return OP_gt;
{geq}       // return OP_geq;
{leq}       // return OP_leq;
{neq}       // return OP_neq;
{eq}        // return OP_eq;
{or}        // return OP_or;
{and}       // return OP_and;
{not}       // return OP_not;
{assign}    // return OP_assign;

{lparen}    // return SP_lparen;
{rparen}    // return SP_rparen;
{lbrack}    // return SP_lbrack;
{rbrack}    // return SP_rbrack;
{lbrace}    // return SP_lbrace;
{rbrace}    // return SP_rbrace;
{comma}     // return SP_comma;
{semicol}   // return SP_semicol;

{iconst}    // return CONST_int;
{fconst}    // return CONST_float; 
{sconst}    // return CONST_string;
{dot}       // return SP_dot;

"return"    // return RS_return;
"typedef"   // return RS_typedef;
"if"        // return RS_if;
"else"      // return RS_else;
"int"       // return RS_int;
"float"     // return RS_float;
"for"       // return RS_for;
"void"      // return RS_void;
"while"     // return RS_while;

{ident}     {
                ptr = lookup(yytext);
                if (ptr == NULL) insertID(yytext);	
                else ptr->counter++;
                // return IDENT;
            }

{error}     printf("ERR : %s\n", yytext); /* return ERROR; */

%%

int main (int argc, char **argv) {
    argc--;
    ++argv;
    if (argc > 0) yyin = fopen(argv[0], "r");
    else yyin = stdin;
    yylex();
    printSymTab();
}

