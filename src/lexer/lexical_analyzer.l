%option noyywrap

%{
/* 声明和选项设置 */
#include "lexical_analyzer.h"

#include <stdio.h>
#include <stdlib.h>

int lines = 1;
int pos_start = 1;
int pos_end = 1;

static void move(len) {
  pos_start = pos_end;
  pos_end = pos_start + len;
}
%}

%%
  /* flex的模式与动作 */
  /* TODO */

  /* Comment and blank handlers. */
"/*"\/*([^*/]|\*[^/]|[^*]\/)*\**"*/" {
  for (int i = 0; i < yyleng; ++i) {
    if (yytext[i] == '\n') {
      ++lines;
      pos_start = 1;
    } else {
      ++pos_start;
    }
  }
  pos_end = pos_start;
  return COMMENT;
}
[\ \f\r\t\v] {
  move(yyleng);
  return BLANK;
}
\n {
  ++lines;
  pos_start = 1;
  pos_end = 1;
  return EOL;
}
<<EOF>> {
  yyterminate();
}

  /* Keywords. */
else {
  move(yyleng);
  return ELSE;
}
if {
  move(yyleng);
  return IF;
}
int {
  move(yyleng);
  return INT;
}
return {
  move(yyleng);
  return RETURN;
}
void {
  move(yyleng);
  return VOID;
}
while {
  move(yyleng);
  return WHILE;
}
float {
  move(yyleng);
  return FLOAT;
}

  /* Op. */
\+ {
  move(yyleng);
  return ADD;
}
- {
  move(yyleng);
  return SUB;
}
\* {
  move(yyleng);
  return MUL;
}
\/ {
  move(yyleng);
  return DIV;
}
\< {
  move(yyleng);
  return LT;
}
\<= {
  move(yyleng);
  return LTE;
}
\> {
  move(yyleng);
  return GT;
}
\>= {
  move(yyleng);
  return GT;
}
== {
  move(yyleng);
  return EQ;
}
!= {
  move(yyleng);
  return NEQ;
}
= {
  move(yyleng);
  return ASSIN;
}

  /* Punct. */
; {
  move(yyleng);
  return SEMICOLON;
}
, {
  move(yyleng);
  return COMMA;
}
\( {
  move(yyleng);
  return LPARENTHESE;
}
\) {
  move(yyleng);
  return RPARENTHESE;
}
\[ {
  move(yyleng);
  return LBRACKET;
}
\] {
  move(yyleng);
  return RBRACKET;
}
\{ {
  move(yyleng);
  return LBRACE;
}
\} {
  move(yyleng);
  return RBRACE;
}

[a-zA-Z]+ {
  move(yyleng);
  return IDENTIFIER;
}
[0-9]+ {
  move(yyleng);
  return INTEGER;
}
([0-9]+\.|[0-9]*\.[0-9]+) {
  move(yyleng);
  return FLOATPOINT;
}
\[\] {
  move(yyleng);
  return ARRAY;
}
[a-zA-Z] {
  move(yyleng);
  return LETTER;
}

. {return ERROR;}
%%
// C代码

/// \brief Analysize a *.cminus file
///
/// \param input_file 需要分析的文件路径
/// \param token Stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h
void analyzer(char *input_file, Token_Node *token_stream) {
  if(!(yyin = fopen(input_file,"r"))) {
    printf("[ERR] No input file\n");
    exit(1);
  }
  printf("[START]: Read from: %s\n", input_file);

  int token;
  int index = 0;

  while(token = yylex()) {
    switch(token) {
      case COMMENT:
        // TODO
      case BLANK:
        // TODO
      case EOL:
        // TODO
        break;
      case ERROR:
        printf("[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
      default:
        if (token == ERROR) {
          sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d line, from %d to %d", yytext, lines,
                  pos_start, pos_end);
        } else {
          strcpy(token_stream[index].text, yytext);
        }
        token_stream[index].token = token;
        token_stream[index].lines = lines;
        token_stream[index].pos_start = pos_start;
        token_stream[index].pos_end = pos_end;
        index++;
        if (index >= MAX_NUM_TOKEN_NODE) {
          printf("%d has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
          exit(1);
        }
    }
  }
  printf("[END]: Analysis completed.\n");
  return;
}
