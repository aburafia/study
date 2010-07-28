#include <stdio.h>
#include <ctype.h>

char *expression;
int e_index = 0;

void parse_expression();
void parse_term();
void parse_int();

char get_token(){
  char token = expression[e_index];
  e_index++;
  return token;
}

void parse_expression(){
  char t;
  parse_term();
  for(;;){
    t = get_token();
    if(t != '+' && t != '-'){
      e_index--; // トークンを押し戻す
      break;
    }
    parse_term();
    if(t == '+'){
      printf("OP_ADD\n");
    }else if(t == '-'){
      printf("OP_SUB\n");
    }
  }
}

void parse_term(){
  char t;
  parse_int();
  for(;;){
    t = get_token();
    if(t != '*' && t != '/'){
      e_index--; // トークンを押し戻す
      break;
    }
    parse_int();
    if(t == '*'){
      printf("OP_MUL\n");
    }else if(t == '/'){
      printf("OP_DIV\n");
    }
  }
}

void parse_int(){
  char t = get_token();
  if(!isdigit(t)){
    printf("文法エラー\n");
    exit(-1);
  }
  printf("OP_PUSH_INT %c\n", t);
}

int main(int argc, char *argv[]){
  expression = expression = argv[1];
  
  parse_expression();

  return 0;
}