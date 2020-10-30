%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines = 1;
int pos_start = 1;
int pos_end = 1;

static void move(int len) {
  pos_start = pos_end;
  pos_end = pos_start + len;
}

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}
%}

%%
  /* Comment and blank handlers. */
"/*"([^*]|\*[^/])*\**"*/" {
  pos_start = pos_end;
  for (int i = 0; i < yyleng; ++i) {
    if (yytext[i] == '\n') {
      ++lines;
      pos_start = 1;
    } else {
      ++pos_start;
    }
  }
  pos_end = pos_start;
}
[\ \f\r\t\v] {
  move(yyleng);
}
\n {
  ++lines;
  pos_start = 1;
  pos_end = 1;
}
<<EOF>> {
  yyterminate();
}

  /* Keywords. */
else {
  move(yyleng);
  pass_node(yytext);
  return ELSE;
}
if {
  move(yyleng);
  pass_node(yytext);
  return IF;
}
int {
  move(yyleng);
  pass_node(yytext);
  return INT;
}
return {
  move(yyleng);
  pass_node(yytext);
  return RETURN;
}
void {
  move(yyleng);
  pass_node(yytext);
  return VOID;
}
while {
  move(yyleng);
  pass_node(yytext);
  return WHILE;
}
float {
  move(yyleng);
  pass_node(yytext);
  return FLOAT;
}

  /* Op. */
\+ {
  move(yyleng);
  pass_node(yytext);
  return ADD;
}
- {
  move(yyleng);
  pass_node(yytext);
  return SUB;
}
\* {
  move(yyleng);
  pass_node(yytext);
  return MUL;
}
\/ {
  move(yyleng);
  pass_node(yytext);
  return DIV;
}
\< {
  move(yyleng);
  pass_node(yytext);
  return LT;
}
\<= {
  move(yyleng);
  pass_node(yytext);
  return LTE;
}
\> {
  move(yyleng);
  pass_node(yytext);
  return GT;
}
\>= {
  move(yyleng);
  pass_node(yytext);
  return GTE;
}
== {
  move(yyleng);
  pass_node(yytext);
  return EQ;
}
!= {
  move(yyleng);
  pass_node(yytext);
  return NEQ;
}
= {
  move(yyleng);
  pass_node(yytext);
  return ASSIN;
}

  /* Punct. */
; {
  move(yyleng);
  pass_node(yytext);
  return SEMICOLON;
}
, {
  move(yyleng);
  pass_node(yytext);
  return COMMA;
}
\( {
  move(yyleng);
  pass_node(yytext);
  return LPARENTHESE;
}
\) {
  move(yyleng);
  pass_node(yytext);
  return RPARENTHESE;
}
\[ {
  move(yyleng);
  pass_node(yytext);
  return LBRACKET;
}
\] {
  move(yyleng);
  pass_node(yytext);
  return RBRACKET;
}
\{ {
  move(yyleng);
  pass_node(yytext);
  return LBRACE;
}
\} {
  move(yyleng);
  pass_node(yytext);
  return RBRACE;
}

[a-zA-Z]+ {
  move(yyleng);
  pass_node(yytext);
  return IDENTIFIER;
}
[0-9]+ {
  move(yyleng);
  pass_node(yytext);
  return INTEGER;
}
([0-9]+\.|[0-9]*\.[0-9]+) {
  move(yyleng);
  pass_node(yytext);
  return FLOATPOINT;
}
\[\] {
  move(yyleng);
  pass_node(yytext);
  return ARRAY;
}

. {
  yyerror(yytext);
}
%%
