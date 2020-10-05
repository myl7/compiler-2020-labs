%option noyywrap

%{
/* 声明和选项设置 */
#include "lexical_analyzer.h"

#include <stdio.h>
#include <stdlib.h>

int lines;
int pos_start;
int pos_end;
%}

%%
  /* flex的模式与动作 */
  /* TODO */

  /* Comment and blank handlers. */
"/*"\/*([^*/]|\*[^/]|[^*]\/)*\**"*/" {return COMMENT;}
[\ \f\r\t\v] {return BLANK;}
\n {return EOL;}
<<EOF>> {yyterminate();}

  /* Keywords. */
else {
  pos_start = pos_end;
  pos_end = pos_start + 4;
  return ELSE;
}
if {
  pos_start = pos_end;
  pos_end = pos_start + 2;
  return IF;
}
int {
  pos_start = pos_end;
  pos_end = pos_start + 3;
  return INT;
}
return {
  pos_start = pos_end;
  pos_end = pos_start + 6;
  return RETURN;
}
void {
  pos_start = pos_end;
  pos_end = pos_start + 4;
  return VOID;
}
while {
  pos_start = pos_end;
  pos_end = pos_start + 5;
  return WHILE;
}
float {
  pos_start = pos_end;
  pos_end = pos_start + 5;
  return FLOAT;
}

  /* Op. */
\+ {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return ADD;
}
- {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return SUB;
}
\* {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return MUL;
}
\/ {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return DIV;
}
\< {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return LT;
}
\<= {
  pos_start = pos_end;
  pos_end = pos_start + 2;
  return LTE;
}
\> {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return GT;
}
\>= {
  pos_start = pos_end;
  pos_end = pos_start + 2;
  return GT;
}
== {
  pos_start = pos_end;
  pos_end = pos_start + 2;
  return EQ;
}
!= {
  pos_start = pos_end;
  pos_end = pos_start + 2;
  return NEQ;
}
= {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return ASSIN;
}

  /* Punct. */
; {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return SEMICOLON;
}
, {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return COMMA;
}
\( {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return LPARENTHESE;
}
\) {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return RPARENTHESE;
}
\[ {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return LBRACKET;
}
\] {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return RBRACKET;
}
\{ {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return LBRACE;
}
\} {
  pos_start = pos_end;
  pos_end = pos_start + 1;
  return RBRACE;
}

[a-zA-Z]+ {
  pos_start = pos_end;
  pos_end = pos_start + strlen(yytext);
  return IDENTIFIER;
}
[0-9]+ {
  pos_start = pos_end;
  pos_end = pos_start + strlen(yytext);
  return INTEGER;
}
([0-9]+\.|[0-9]+\.[0-9]+) {
  pos_start = pos_end;
  pos_end = pos_start + strlen(yytext);
  return FLOATPOINT;
}
\[\] {
  pos_start = pos_end;
  pos_end = pos_start + 2;
  return ARRAY;
}
[a-zA-Z] {
  pos_start = pos_end;
  pos_end = pos_start + 1;
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
        ;
        int yylen = strlen(yytext);
        for (int i = 0; i < yylen; ++i) {
          if (yytext[i] == '\n') {
            ++lines;
            pos_start = 0;
          } else {
            ++pos_start;
          }
        }
        pos_end = pos_start + 1;
        break;
      case BLANK:
        pos_start = pos_end;
        pos_end = pos_start + 1;
        break;
      case EOL:
        ++lines;
        pos_start = 0;
        pos_end = 1;
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
