#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "token.h"

char *g_string_pointer_pool[65536];
int g_string_pointer_pool_size = 0;

static FILE *st_source_file;
static int st_current_line_number;

typedef enum {
  INITIAL_STATE,
  INT_VALUE_STATE,
  IDENTIFIER_STATE,
  STRING_STATE,
  OPERATOR_STATE,
  COMMENT_STATE
} LexerState;

static void lex_error(char *message, int ch){
  fprintf(stderr, "lex error:%s near\'%c\'\n",
    message, ch);
  exit(1);
}

static char* my_strdup(char *src){
  char *dest = malloc(strlen(src) + 1);
  g_string_pointer_pool[g_string_pointer_pool_size++]
    = dest;
  strcpy(dest, src);
  return dest;
}

void lex_initialize(FILE *src_fp){
  st_source_file = src_fp;
  st_current_line_number = 1;
}

/**
*	現在組み立て中のトークンに1つ足す。
*	'\0'はNULLで終端文字として認識されるのね。なるほど。
*/
static void add_letter(char *token, int letter){
  int len = strlen(token);
  token[len] = letter;
  token[len+1] = '\0';
}

typedef struct {
  char *token;
  TokenKind kind;
} OperatorInfo;

static OperatorInfo st_operator_table[] = {
  {"==", EQ_TOKEN},
  {"!=", NE_TOKEN},
  {">=", GE_TOKEN},
  {"<=", LE_TOKEN},
  {"+", ADD_TOKEN},
  {"-", SUB_TOKEN},
  {"*", MUL_TOKEN},
  {"/", DIV_TOKEN},
  {"=", ASSIGN_TOKEN},
  {">", GT_TOKEN},
  {"<", LT_TOKEN},
  {"(", LEFT_PAREN_TOKEN},
  {")", RIGHT_PAREN_TOKEN},
  {"{", LEFT_BRACE_TOKEN},
  {"}", RIGHT_BRACE_TOKEN},
  {",", COMMA_TOKEN},
  {";", SEMICOLON_TOKEN},
};


int in_operator(char *token, int letter){
  int op_idx;
  int letter_idx;
  int len = strlen(token);

  //演算子テーブルの一覧走査
  for(op_idx = 0; op_idx < (sizeof(st_operator_table)
    / sizeof(OperatorInfo)); op_idx++){
	
	//複数文字の演算子の文字列走査(=!とか==)
    for(letter_idx = 0; letter_idx < len &&
      st_operator_table[op_idx].token[letter_idx]!='\0';
      letter_idx++){
		
      //一致していなければ次の演算子を評価しに行く(*1)
      if(token[letter_idx]
        != st_operator_table[op_idx].token[letter_idx]){
          break;
      }
    }
    
    //最後に評価したところが一致してればTrue
    //(*1)のところでbreakしても、絶対ここは通らない。
    if(token[letter_idx]=='\0' &&
    st_operator_table[op_idx].token[letter_idx]==letter){
      return 1;
    }
  }
  return 0;
}

TokenKind select_operator(char *token){
  int i;
  for(i = 0; i < sizeof(st_operator_table)
    / sizeof(OperatorInfo); i++){
    if(!strcmp(token, st_operator_table[i].token)){
      return st_operator_table[i].kind;
    }
  }
  assert(0);
  return 0;
}

typedef struct {
  char *token;
  TokenKind kind;
} KeywordInfo;

KeywordInfo st_keyword_table[] = {
  {"if", IF_TOKEN},
  {"else", ELSE_TOKEN},
  {"while", WHILE_TOKEN},
  {"goto", GOTO_TOKEN},
  {"gosub", GOSUB_TOKEN},
  {"return", RETURN_TOKEN},
  {"print", PRINT_TOKEN},
};

int is_keyword(char *token, TokenKind *kind){
  int i;
  for(i = 0; i < sizeof(st_keyword_table)
    / sizeof(KeywordInfo); i++){
    if(!strcmp(st_keyword_table[i].token, token)){
      *kind = st_keyword_table[i].kind;
      return 1;
    }
  }
  return 0;
}

/**
*	ここがparserから呼ばれる。
*	if(a==b){
*	だと
*	[if] [(] [a] [==] [b] [{]
*	がトークンとなり、lex_get_tokenが呼ばれるたびにひとつづつ返す。
*/
Token lex_get_token(void){
  Token ret;
  LexerState state = INITIAL_STATE;
  char token[256];
  int ch;
    
  token[0] = '\0';	//とりあえず0文字の文字列
  while ((ch = getc(st_source_file)) != EOF){
    switch (state){
		
	//まずはここが動く。改行があった場合もリセットとみなす。
    case INITIAL_STATE:
      if(isdigit(ch)){  // 数字？
        add_letter(token, ch);
        state = INT_VALUE_STATE;
      }else if(isalpha(ch) || ch == '_'){

		// 英文字？ _？
		//変数名やifなんかは数字から始まっちゃ駄目ってことだね。
		//あと演算子で使う文字も使えない
        add_letter(token, ch);
        state = IDENTIFIER_STATE;

      }else if(ch == '\"'){ // 文字列？
        state = STRING_STATE;
      }else if(in_operator(token, ch)){ // 演算子？
      
        //此処一見簡単だけど、==とか=!とかはどう見てるの？って思った。
        //だけど、2文字使う演算子も、演算子で使う文字列で表現されてるから
        //ここを通るんだ。「= - + * !」なんかで造られてるものね。[== =- =+]とかね。
        add_letter(token, ch);
        state = OPERATOR_STATE;

      }else if(isspace(ch)){ // 空白？
        if(ch == '\n'){ // 改行？
          st_current_line_number++;
        }
      }else if(ch == '#'){ // コメント？
        state = COMMENT_STATE;
      }else{
        lex_error("bad character", ch); // エラー
      }
      break;
      
    case INT_VALUE_STATE:
      if (isdigit(ch)) {
        add_letter(token, ch);
      } else {
        ret.kind = INT_VALUE_TOKEN;
        sscanf(token, "%d", &ret.u.int_value);
        ungetc(ch, st_source_file);
        goto LOOP_END;
      }
      break;
      
      
      
    //[a-z] _ が来た時の処理。
    case IDENTIFIER_STATE:
      //変数やifなどの2文字目以降は数字がはいってもOKってことか。
      if(isalpha(ch) || ch == '_' || isdigit(ch)){	
        add_letter(token, ch);
      }else{

        //空白とか(だった場合、トークンの終了ってこと。
        //積み上げてたトークン文字列を含む構造体をreturnで返す。
        //あと、読んでしまった文字分、1バイトファイルの読み込みポインタの位置を戻す

        ret.u.identifier = token;
        ungetc(ch, st_source_file);
        goto LOOP_END;
      }
      break;
    case STRING_STATE:
      if (ch == '\"') {
        ret.kind = STRING_LITERAL_TOKEN;
        ret.u.string = my_strdup(token);
        goto LOOP_END;
      } else {
        add_letter(token, ch);
      }
      break;
    case OPERATOR_STATE:
      if (in_operator(token, ch)) {
        add_letter(token, ch);
      } else {
        ungetc(ch, st_source_file);
        goto LOOP_END;
      }
      break;
      
    //改行があるまでずーっとCOMMENT_STATE
    case COMMENT_STATE:
      if (ch == '\n') {
        state = INITIAL_STATE;
      }
      break;
    default:
      assert(0);
    }
  }
 LOOP_END:
  if (ch == EOF) {
    if (state == INITIAL_STATE
         || state == COMMENT_STATE) {
      ret.kind = END_OF_FILE_TOKEN;
      return ret;
    }
  }
  if (state == IDENTIFIER_STATE) {
    if (!is_keyword(token, &ret.kind)) {
      ret.kind = IDENTIFIER_TOKEN;
      ret.u.string = my_strdup(token);
    }
  } else if (state == OPERATOR_STATE) {
    ret.kind = select_operator(token);
  }
  return ret;
}

int lex_get_line_number(void){
  return st_current_line_number;
}
