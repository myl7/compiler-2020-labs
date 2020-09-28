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
[^\f\r\t\v] {return BLANK;}
\n {return EOL;}
<<EOF>> {yyterminate();}

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
        break;
      case BLANK:
        ++pos_start;
        break;
      case EOL:
        ++lines;
        pos_start = 0;
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
