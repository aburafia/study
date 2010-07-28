#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED
#include <stdio.h>

typedef enum {
  INT_VALUE_TOKEN,     // ®”
  IDENTIFIER_TOKEN,    // Ž¯•ÊŽq
  STRING_LITERAL_TOKEN,// •¶Žš—ñ
  EQ_TOKEN,            // ==
  NE_TOKEN,            // !=
  GE_TOKEN,            // >=
  LE_TOKEN,            // <=
  ADD_TOKEN,           // +
  SUB_TOKEN,           // -
  MUL_TOKEN,           // *
  DIV_TOKEN,           // /
  ASSIGN_TOKEN,        // =
  GT_TOKEN,            // >
  LT_TOKEN,            // <
  LEFT_PAREN_TOKEN,    // (
  RIGHT_PAREN_TOKEN,   // )
  LEFT_BRACE_TOKEN,    // {
  RIGHT_BRACE_TOKEN,   // }
  COMMA_TOKEN,         // ,
  SEMICOLON_TOKEN,     // ;
  IF_TOKEN,            // if
  ELSE_TOKEN,          // else
  WHILE_TOKEN,         // while
  GOTO_TOKEN,          // goto
  GOSUB_TOKEN,         // gosub
  RETURN_TOKEN,        // return
  PRINT_TOKEN,         // print
  END_OF_FILE_TOKEN    // EOF
} TokenKind;

typedef struct {
  TokenKind  kind;
  union {
    int int_value;
    char *string;
    char *identifier;
  } u;
} Token;

void lex_initialize(FILE *src_fp);
Token lex_get_token(void);
int lex_get_line_number(void);

extern char *g_string_pointer_pool[];

#endif /* TOKEN_H_INCLUDED */
