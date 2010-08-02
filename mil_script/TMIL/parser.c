#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "token.h"
#include "mil.h"

int g_bytecode[65536];
int g_bytecode_size = 0;
char *g_str_pool[4096];
int g_str_pool_size = 0;

static char *st_var_table[VAR_MAX];
static int st_var_table_size = 0;

typedef struct {
  char *identifier;
  int address;
} Label;
static Label st_label_table[65536];
static int st_label_table_size = 0;

static Token st_look_ahead_token;
static int st_look_ahead_token_exists;

static Token get_token(void){
  Token ret;

	char dbg[100];

  if (st_look_ahead_token_exists) {
    ret = st_look_ahead_token;
    st_look_ahead_token_exists = 0;

	sprintf(dbg, "gettoken/exist kind=%d", ret.kind);

  } else {
    ret = lex_get_token();

	sprintf(dbg, "gettoken kind=%d", ret.kind);

  }

  lg(dbg);

  return ret;
}

/**
 *トークンを一つ戻す。っていうか戻したトークンっていう置き場をつくってそこに格納。
 *トークンを取得する関数は置き場にあれば置き場から、そうでなければちゃんと取得し直す
 *みたいな処理
 */
static void unget_token(Token token){
  st_look_ahead_token = token;
  st_look_ahead_token_exists = 1;
}

static void add_bytecode(int bytecode){
  g_bytecode[g_bytecode_size] = bytecode;
  g_bytecode_size++;
}

static void parse_error(char *message){
  int line_number = lex_get_line_number();
  fprintf(stderr, "%d:Parse error:%s\n",
   line_number, message);
  exit(1);
}

static void check_expected_token(TokenKind expected){
  Token token = get_token();
  if (token.kind != expected) {
    parse_error("parse error");
  }
}

/**
 * 登録した変数管理配列から、名称で変数を探す
 */
static int search_var(char *identifier){
  int i;
  for (i = 0; i < st_var_table_size; i++) {
    if (!strcmp(identifier, st_var_table[i])) //strcmpは一致すると0。ってことで一致するとここが通る
      return i;
  }
  return -1;
}

/**
 * 登録した変数管理配列から変数を探す。
 * 無い場合は登録する
 */
static int search_or_new_var(char *identifier){
  int ret;
  ret = search_var(identifier);
  if (ret < 0) {
    st_var_table[st_var_table_size] = identifier;
    return st_var_table_size++;
  }
  return ret;
}

static void parse_expression(void);

/**
 * 一番優先度の高い演算処理。
 * 文字、数値や、変数など
 * あとは（がある場合は再度式の評価を最低優先度から評価して行く(parse_expression）。
 */
static void parse_primary_expression(void){
  Token token;

  lg("parse_primary_expressio");

  token = get_token();
  if (token.kind == INT_VALUE_TOKEN) {
    add_bytecode((int)OP_PUSH_INT);
    add_bytecode(token.u.int_value);
  } else if (token.kind == STRING_LITERAL_TOKEN) {

    //文字列は別途配列にて管理　　　
    add_bytecode((int)OP_PUSH_STRING);
    g_str_pool[g_str_pool_size] = token.u.string;
    add_bytecode(g_str_pool_size);
    g_str_pool_size++;
  } else if (token.kind == LEFT_PAREN_TOKEN) {
    parse_expression();
    check_expected_token(RIGHT_PAREN_TOKEN); //)で終わるか確認
  } else if (token.kind == IDENTIFIER_TOKEN) {
    int var_idx = search_var(token.u.identifier);
    if (var_idx < 0) {
      parse_error("identifier not found.");
    }
    add_bytecode((int)OP_PUSH_VAR);
    add_bytecode(var_idx);
  }
}

/**
 * マイナス値だった場合の処理。
 * より高い優先度の演算を行った後に
 * 多分-1を掛けるとか言う
 * バイトコード入れるんだと思う
 */
static void parse_unary_expression(void){

	lg("parse_unary_expressio");

  Token token;
  token = get_token();
  if (token.kind == SUB_TOKEN) {
    parse_primary_expression();
    add_bytecode((int)OP_MINUS);
  } else {
    unget_token(token);
    parse_primary_expression();
  }
}

/**
 *乗算所算計処理
 *その前により高い演算の処理へJUMP
 *
 */
static void parse_multiplicative_expression(void){

	lg("parse_multiplicative_expressio");

  Token token;
  parse_unary_expression();
  for (;;) {
    token = get_token();
    if (token.kind != MUL_TOKEN
      && token.kind != DIV_TOKEN) {
      unget_token(token);
      break;
    }
    parse_unary_expression();  //マイナス値かもしれないから、その処理
    if (token.kind == MUL_TOKEN) {
      add_bytecode((int)OP_MUL);
    } else {
      add_bytecode((int)OP_DIV);
    }
  }
}

/**
 *たすひく系処理
 *その前により優先度の高い演算処理へJUMPする
 *
 */
static void parse_additive_expression(void){

	lg("parse_additive_expressio");

  Token token;
  parse_multiplicative_expression();
  for (;;) {
    token = get_token();
    if (token.kind != ADD_TOKEN
      && token.kind != SUB_TOKEN) {
      unget_token(token);
      break;
    }
    parse_multiplicative_expression(); //マイナス値かもしれないから、その処理
    if (token.kind == ADD_TOKEN) {
      add_bytecode((int)OP_ADD);
    } else {
      add_bytecode((int)OP_SUB);
    }
  }
}

/**
 *比較系演算の処理部分
 *でも比較系演算の処理のまえに、足す引くの処理を行う。
 *つまりタス引くの演算子の優先度が低いってこと
 *
 *比較系演算子でなければ取得したトークンを戻して
 *比較系ならば、対応したバイトコードを出力して終わり
 */
static void parse_compare_expression(void){

	lg("parse_compare_expressio");

  Token token;
  parse_additive_expression();
  for (;;) {
    token = get_token();
    if (token.kind != EQ_TOKEN
      && token.kind != NE_TOKEN
      && token.kind != GT_TOKEN
      && token.kind != GE_TOKEN
      && token.kind != LT_TOKEN
      && token.kind != LE_TOKEN) {
      unget_token(token);
      break;
    }
    parse_additive_expression();
    if (token.kind == EQ_TOKEN) {
      add_bytecode((int)OP_EQ);
    } else if (token.kind == NE_TOKEN) {
      add_bytecode((int)OP_NE);
    } else if (token.kind == GT_TOKEN) {
      add_bytecode((int)OP_GT);
    } else if (token.kind == GE_TOKEN) {
      add_bytecode((int)OP_GE);
    } else if (token.kind == LT_TOKEN) {
      add_bytecode((int)OP_LT);
    } else if (token.kind == LE_TOKEN) {
      add_bytecode((int)OP_LE);
    }
  }
}

/**
 *式の評価の関数ここからは関数の串刺し。
 *だけど、それは演算の評価順序を構築するため
 *まずは比較系演算処理へJUMP.
 *んで比較系を見る前にもっと優先度のたかい演算子の処理へJUMPするから
 *結局優先度が一番低い処理っていう。
 */
static void parse_expression(void){
  parse_compare_expression();
}

static void parse_block(void);

static int get_label(void){
  return st_label_table_size++;
}

static void set_label(int label_idx){
  st_label_table[label_idx].address
    = g_bytecode_size;
}

static int search_or_new_label(char *label){
  int i;
  for (i = 0; i < st_label_table_size; i++) {
    if (st_label_table[i].identifier != NULL
      && !strcmp(st_label_table[i].identifier,
      label)){
      return i;
    }
  }
  st_label_table[i].identifier = label;
  return st_label_table_size++;
}

/**
 *ifってトークンがきたら呼ばれる
 *
 */
static void parse_if_statement(void){

	lg("parse_if_statemen");

  Token token;
  int else_label;
  int end_if_label;

  check_expected_token(LEFT_PAREN_TOKEN); //  (かどうか
  parse_expression(); //式
  check_expected_token(RIGHT_PAREN_TOKEN); // )かどうか

  else_label = get_label();
  add_bytecode((int)OP_JUMP_IF_ZERO);
  add_bytecode(else_label);
  parse_block();
  token = get_token();
  if (token.kind == ELSE_TOKEN) { 
    end_if_label = get_label();
    add_bytecode((int)OP_JUMP);
    add_bytecode(end_if_label);
    set_label(else_label);
    parse_block();
    set_label(end_if_label);
  } else {
    unget_token(token);
    set_label(else_label);
  }
}

static void parse_while_statement(void){

	lg("parse_while_statemen");

  int loop_label;
  int end_while_label;

  loop_label = get_label();
  set_label(loop_label);

  check_expected_token(LEFT_PAREN_TOKEN);
  parse_expression();
  check_expected_token(RIGHT_PAREN_TOKEN);

  end_while_label = get_label();
  add_bytecode((int)OP_JUMP_IF_ZERO);
  add_bytecode(end_while_label);
  parse_block();
  add_bytecode((int)OP_JUMP);
  add_bytecode(loop_label);
  set_label(end_while_label);
}

static void parse_print_statement(void){

	lg("parse_print_statemen");

  check_expected_token(LEFT_PAREN_TOKEN);
  parse_expression();
  check_expected_token(RIGHT_PAREN_TOKEN);
  add_bytecode((int)OP_PRINT);
  check_expected_token(SEMICOLON_TOKEN);
}

static void parse_assign_statement(char *identifier){

	lg("parse_assign_statemen");

  int var_idx = search_or_new_var(identifier);

  //たしかにこの次は代入演算子だよね
  check_expected_token(ASSIGN_TOKEN);
  parse_expression();
  add_bytecode((int)OP_ASSIGN_TO_VAR);
  add_bytecode(var_idx);
  check_expected_token(SEMICOLON_TOKEN);
}

static void parse_goto_statement(void){

	lg("parse_goto_statemen");

  Token token;
  int label;

  check_expected_token(MUL_TOKEN);
  token = get_token();
  if (token.kind != IDENTIFIER_TOKEN) {
    parse_error("label identifier expected");
  }
  label = search_or_new_label(token.u.identifier);
  add_bytecode((int)OP_JUMP);
  add_bytecode(label);
  check_expected_token(SEMICOLON_TOKEN);
}

static void parse_gosub_statement(void){

	lg("parse_gosub_statemen");

  Token token;
  int label;

  check_expected_token(MUL_TOKEN);
  token = get_token();
  if (token.kind != IDENTIFIER_TOKEN) {
    parse_error("label identifier expected");
  }
  label = search_or_new_label(token.u.identifier);
  add_bytecode((int)OP_GOSUB);
  add_bytecode(label);
  check_expected_token(SEMICOLON_TOKEN);
}

static void parse_label_statement(void){

	lg("parse_label_statemen");

  Token token;
  int label;

  token = get_token();
  if (token.kind != IDENTIFIER_TOKEN) {
    parse_error("label identifier expected");
  }
  label = search_or_new_label(token.u.identifier);
  set_label(label);
}

static void parse_return_statement(void){

	lg("parse_return_statemen");

  add_bytecode((int)OP_RETURN);
  check_expected_token(SEMICOLON_TOKEN);
}

/**
 *一番最初の評価関数
 *
 */
static void parse_statement(void){
  Token token;
  token = get_token();

  if (token.kind == IF_TOKEN) {

	  lg("IF_TOKEN");

    parse_if_statement();
  } else if (token.kind == WHILE_TOKEN) {

	  lg("WHILE_TOKEN");

    parse_while_statement();
  } else if (token.kind == PRINT_TOKEN) {

	  lg("PRINT_TOKEN");

    parse_print_statement();
  } else if (token.kind == GOTO_TOKEN) {

	  lg("GOTO_TOKEN");

    parse_goto_statement();
  } else if (token.kind == GOSUB_TOKEN) {

	  lg("GOSUB_TOKNE");

    parse_gosub_statement();
  } else if (token.kind == RETURN_TOKEN) {

	  lg("RETURN_TOKEN");

    parse_return_statement();
  } else if (token.kind == MUL_TOKEN) {

	  lg("MUL_TOKEN");

    parse_label_statement();
  } else if (token.kind == IDENTIFIER_TOKEN) {

	  lg("IDENTIFIER_TOKEN");
	  //なるほど、変数の次は代入演算子だからか。
    parse_assign_statement(token.u.identifier);
  } else {

	  lg("bad statement token");

    parse_error("bad statement.");
  }
}

static void parse_block(void){
  Token token;
  check_expected_token(LEFT_BRACE_TOKEN);
  for (;;) {
    token = get_token();
    if (token.kind == RIGHT_BRACE_TOKEN) {
      break;
    }
    unget_token(token);
    parse_statement();
  }
}

static void fix_labels(void){
  int i;
  for (i = 0; i < g_bytecode_size; i++) {
    if (g_bytecode[i] == OP_PUSH_INT
      || g_bytecode[i] == OP_PUSH_STRING
      || g_bytecode[i] == OP_PUSH_VAR
      || g_bytecode[i] == OP_ASSIGN_TO_VAR) {
      i++;
    } else if (g_bytecode[i] == OP_JUMP
      || g_bytecode[i] == OP_JUMP_IF_ZERO
      || g_bytecode[i] == OP_GOSUB) {
      g_bytecode[i+1]
      = st_label_table[g_bytecode[i+1]].address;
    }
  }
}

/**
 *ここがパーサーの入り口
 */
static void parse(void){
  Token token;

  for (;;) {
	  lg("評価はじめ");

    token = get_token();
    if (token.kind == END_OF_FILE_TOKEN) {
      break;
    } else {
      unget_token(token);
      parse_statement();
    }
  }
}

int main(int argc, char **argv){
  FILE *src_fp;
  int c = 0;

  if (argc != 2) {
    fprintf(stderr, "Usage:%s filename\n",
     argv[0]);
    exit(1);
  }
  src_fp = fopen(argv[1], "r");
  if (src_fp == NULL) {
    fprintf(stderr, "%s not found.\n", argv[1]);
    exit(1);
  }
  lex_initialize(src_fp);

  parse();
  fix_labels();
  mvm_execute();

  while(g_string_pointer_pool[c] != 0){
    free(g_string_pointer_pool[c++]);
  }
  return 0;
}


int lg(char *c){
  fprintf(stderr, "dbg:%s\n", c);
  return 0;
}

