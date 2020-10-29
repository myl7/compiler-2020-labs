%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "syntax_tree.h"

// external functions from lex
extern int yylex();

// external variables from lexical_analyzer module
extern int lines;
extern char *yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree
syntax_tree *gt;

// Error reporting
void yyerror(const char *s);

// Helper functions written for you with love
syntax_tree_node *node(const char *node_name, int children_num, ...);
%}

/* TODO: Complete this definition. */
%union {}

/* TODO: Your tokens here. */

%start program

%%
program: declaration-list {$$ = node("program", 1, $1); gt->root = $$;}
;
declaration-list:
  declaration-list declaration {$$ = node("declaration-list", 2, $1, $2);}
| declaration {$$ = node("declaration-list", 1, $1);}
;
declaration:
  var-declaration {$$ = node("declaration", 1, $1);}
| fun-declaration {$$ = node("declaration", 1, $1);}
;
var-declaration:
  type-specifier ID ';' {$$ = node("var-declaration", 3, $1, node($2, 0), node(";", 0));};
| type-specifier ID '[' INTEGER ']' ';' {$$ = node("var-declaration", 6, $1, node($2, 0), node("[", 0), $3, node("]", 0), node(";", 0));}
;
type-specifier:
  'int' {$$ = node("type-specifier", 1, node("int", 0));}
| 'float' {$$ = node("type-specifier", 1, node("float", 0));}
| 'void' {$$ = node("type-specifier", 1, node("void", 0));}
;
fun-declaration: type-specifier ID '(' params ')' compound-stmt {$$ = node("fun-declaration", 6, $1, node($2, 0), node("(", 0), $3, node(")", 0), $4);}
;
params:
  param-list {$$ = node("params", 1, $1);}
| 'void' {$$ = node("params", 1, node("void", 0));}
;
param-list:
  param-list ',' param {$$ = node("param-list", 3, $1, node(",", 0), $2);}
∣ param {$$ = node("param-list", 1, $1);}
;
param:
  type-specifier ID {$$ = node("param", 2, $1, node($2, 0));}
∣ type-specifier ID '[' ']' {$$ = node("param", 4, $1, node($2), node("[", 0), node("]", 0));}
;
compound-stmt: '{' local-declarations statement-list '}' {$$ = node("compound-stmt", 4, node("(", 0), $1, $2, node(")", 0));}
;
local-declarations:
  local-declarations var-declaration {$$ = node("local-declarations", 2, $1, $2);}
∣ {$$ = node("local-declarations", 0);}
;
statement-list:
  statement-list statement {$$ = node("statement-list", 2, $1, $2);}
∣ {$$ = node("statement-list", 0);}
;
statement:
  expression-stmt {$$ = node("statement", 1, $1);}
∣ compound-stmt {$$ = node("statement", 1, $1);}
∣ selection-stmt {$$ = node("statement", 1, $1);}
∣ iteration-stmt {$$ = node("statement", 1, $1);}
∣ return-stmt {$$ = node("statement", 1, $1);}
​;
expression-stmt:
  expression ';' {$$ = node("expression-stmt", 2, $1, node(";", 0));}
∣ ';' {$$ = node("expression-stmt", 1, node(";", 0));}
;
selection-stmt:
  'if' '(' expression ')' statement {$$ = node("selection-stmt", 5, node("if", 0), node("(", 0), $1, node(")", 0), $2);}
∣ 'if' '(' expression ')' statement ELSE statement {$$ = node("selection-stmt", 7, node("if", 0), node("(", 0), $1, node(")", 0), $2, node("else", 0), $3);}
​;
iteration-stmt: 'while' '(' expression ')' statement {$$ = node("iteration-stmt", 5, node("while", 0), node("(", 0), $1, node(")", 0), $2);}
;
return-stmt:
  'return' ';' {$$ = node("return-stmt", 2, node("return", 0), node(";", 0));}
∣ 'return' expression ';' {$$ = node("return-stmt", 3, node("return", 0), $1, node(";", 0));}
;
expression:
  var '=' expression {$$ = node("expression", 3, $1, node("=", 0), $2);}
∣ simple-expression {$$ = node("expression", 1, $1);}
;
var:
  ID {$$ = node("var", 1, node($1, 0));}
| ID [ expression ] {$$ = ("var", 4, node($1, 0), node("[", 0), $2, node("]", 0));}
;
simple-expression:
  additive-expression relop additive-expression {$$ = node("simple-expression", 3, $1, $2, $3);}
∣ additive-expression {$$ = node("simple-expression", 1, $1);}
;
relop:
 '<=' {$$ = node("relop", 1, node("<=", 0));}
∣ '<' {$$ = node("relop", 1, node("<", 0));}
∣ '>' {$$ = node("relop", 1, node(">", 0));}
∣ '>=' {$$ = node("relop", 1, node(">=", 0));}
∣ '==' {$$ = node("relop", 1, node("==", 0));}
∣ '!=' {$$ = node("relop", 1, node("!=", 0));}
;
additive-expression:
  additive-expression addop term {$$ = node("additive-expression", 3, $1, $2, $3);}
∣ term {$$ = ndoe("additive-expression", 1, $1);}
;
addop:
  '+' {$$ = node("addop", 1, node("+", 0));}
| '-' {$$ = node("addop", 1, node("-", 0));}
;
term:
  term mulop factor {$$ = node("term", 3, $1, $2, $3);}
∣ factor {$$ = node("term", 1, $1);}
;
mulop:
  '*' {$$ = ndoe("mulop", 1, node("*", 0));}
| '/' {$$ = ndoe("mulop", 1, node("/", 0));}
;
factor:
  '(' expression ')' {$$ = node("factor", 3, node("(", 0), $1, node(")", 0));}
| var {$$ = node("factor", 1, $1);}
| call {$$ = node("factor", 1, $1);}
| integer {$$ = node("factor", 1, $1);}
| float {$$ = node("factor", 1, $1);}
;
integer: INTEGER {$$ = node("integer", 1, node($1, 0));}
;
float: FLOATPOINT {$$ = node("float", 1, node($1, 0));}
;
call: ID '(' args ')' {$$ = node("call", 4, node($1, 0), node("(", 0), $1, node(")", 0));}
;
args:
  arg-list {$$ = node("args", 1, $1);}
| {$$ = node("args", 0);}
;
arg-list:
  arg-list ',' expression {$$ = node("arg-list", 3, $1, node(",", 0), $2);}
| expression {$$ = node("arg-list", 1, $1);}
;
%%

/// The error reporting function.
void yyerror(const char *s)
{
    // TO STUDENTS: This is just an example.
    // You can customize it as you like.
    fprintf(stderr, "error at line %d column %d: %s\n", lines, pos_start, s);
}

/// Parse input from file `input_path`, and prints the parsing results
/// to stdout.  If input_path is NULL, read from stdin.
///
/// This function initializes essential states before running yyparse().
syntax_tree *parse(const char *input_path)
{
    if (input_path != NULL) {
        if (!(yyin = fopen(input_path, "r"))) {
            fprintf(stderr, "[ERR] Open input file %s failed.\n", input_path);
            exit(1);
        }
    } else {
        yyin = stdin;
    }

    lines = pos_start = pos_end = 1;
    gt = new_syntax_tree();
    yyrestart(yyin);
    yyparse();
    return gt;
}

/// A helper function to quickly construct a tree node.
///
/// e.g.
///     $$ = node("program", 1, $1);
///     $$ = node("local-declarations", 0);
syntax_tree_node *node(const char *name, int children_num, ...)
{
    syntax_tree_node *p = new_syntax_tree_node(name);
    syntax_tree_node *child;
    if (children_num == 0) {
        child = new_syntax_tree_node("epsilon");
        syntax_tree_add_child(p, child);
    } else {
        va_list ap;
        va_start(ap, children_num);
        for (int i = 0; i < children_num; ++i) {
            child = va_arg(ap, syntax_tree_node *);
            syntax_tree_add_child(p, child);
        }
        va_end(ap);
    }
    return p;
}
