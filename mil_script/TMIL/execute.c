#include <stdio.h>
#include <assert.h>
#include "mil.h"

#define STACK_SIZE_MAX (65536)

typedef enum {
  INT_VALUE_TYPE,
  STRING_VALUE_TYPE
} ValueType;

typedef struct {
  ValueType type;
  union {
    int int_value;
    char *string_value;
  } u;
} Value;

static Value st_stack[STACK_SIZE_MAX];
static Value st_variable[VAR_MAX];

void mvm_execute(void){
  lg("vm評価開始");

  int pc = 0;
  int sp = 0;

  while (pc < g_bytecode_size) {
    switch (g_bytecode[pc]) {
    case OP_PUSH_INT:
      fprintf(stderr, "OP_PUSH_INT pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp].type = INT_VALUE_TYPE;
      st_stack[sp].u.int_value
        = (int)g_bytecode[pc+1];
      sp++;
      pc += 2;
      
      exeprint();
      
      break;
    case OP_PUSH_STRING:
      fprintf(stderr, "OP_PUSH_STRING pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp].type = STRING_VALUE_TYPE;
      st_stack[sp].u.string_value
        = g_str_pool[g_bytecode[pc+1]];
      sp++;
      pc += 2;

      exeprint();

      break;
    case OP_ADD:
      fprintf(stderr, "OP_ADD pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        + st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      
      exeprint();

      break;
    case OP_SUB:
      fprintf(stderr, "OP_SUB pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        - st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_MUL:
      fprintf(stderr, "OP_MUL pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        * st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_DIV:
      fprintf(stderr, "OP_DIV pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        / st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_MINUS:
      fprintf(stderr, "OP_MINUS pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-1].u.int_value
        = -st_stack[sp-1].u.int_value;
      pc++;

      exeprint();

      break;
    case OP_EQ:
      fprintf(stderr, "OP_EQ pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        == st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_NE:
      fprintf(stderr, "OP_NE pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        != st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_GT:
      fprintf(stderr, "OP_GT pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        > st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_GE:
      fprintf(stderr, "OP_GE pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        >= st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_LT:
      fprintf(stderr, "OP_LT pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        < st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_LE:
      fprintf(stderr, "OP_LE pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        <= st_stack[sp-1].u.int_value;
      sp--;
      pc++;

      exeprint();

      break;
    case OP_PUSH_VAR:
      fprintf(stderr, "OP_PUSH_VAR pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp]
        = st_variable[(int)g_bytecode[pc+1]];
      sp++;
      pc += 2;

      exeprint();

      break;
    case OP_ASSIGN_TO_VAR:
      fprintf(stderr, "OP_ASSIGN_TO_VAR pc[%d] sp[%d]\n",pc, sp);

      st_variable[(int)g_bytecode[pc+1]]
        = st_stack[sp-1];
      sp--;
      pc += 2;

      exeprint();

      break;
    case OP_JUMP:
      fprintf(stderr, "OP_JUMP pc[%d] sp[%d]\n",pc, sp);

      pc = (int)g_bytecode[pc+1];

      exeprint();

      break;
    case OP_JUMP_IF_ZERO:
      fprintf(stderr, "OP_JUMP_IF_ZERO pc[%d] sp[%d]\n",pc, sp);

      if (!st_stack[sp-1].u.int_value) {
        pc = (int)g_bytecode[pc+1];
      } else {
        pc += 2;
      }
      sp--;

      exeprint();

      break;
    case OP_GOSUB:
      fprintf(stderr, "OP_GOSUB pc[%d] sp[%d]\n",pc, sp);

      st_stack[sp].u.int_value = pc + 2;
      sp++;
      pc = (int)g_bytecode[pc+1];

      exeprint();

      break;
    case OP_RETURN:
      fprintf(stderr, "OP_RETURN pc[%d] sp[%d]\n",pc, sp);

      pc = st_stack[sp-1].u.int_value;
      sp--;

      exeprint();

      break;
    case OP_PRINT:
      fprintf(stderr, "OP_PRINT pc[%d] sp[%d]\n",pc, sp);

      if(st_stack[sp-1].type == INT_VALUE_TYPE){
        printf("%d\n", st_stack[sp-1].u.int_value);
      }else if(st_stack[sp-1].type == STRING_VALUE_TYPE){
        printf("%s\n", st_stack[sp-1].u.string_value);
      }
      sp--;
      pc++;

      exeprint();

      break;
    default:
      assert(0);
    }
  }
}

int exeprint(void){

  int i;

  fprintf(stderr, "========\n");

  fprintf(stderr, "pc:%d bytecode:%d\n",pc, g_bytecode[pc]);


  fprintf(stderr, "  ---->sta\n");

  //スタックの出力
  for(i = 0; i < sizeof(st_stack) / sizeof(Value); i++){
	
	if(st_stack[i].type == INT_VALUE_TYPE){
		fprintf(stderr, "  int type:%d\n", st_stack[i].u.int_value);
	}else{
		fprintf(stderr, "  str type:%s\n", st_stack[i].u.string_value);
	}
  }

  fprintf(stderr, " ---->st_variable\n");

  //バリアぶるの出力
  for(i = 0; i < sizeof(st_variable) / sizeof(Value); i++){

	fprintf(stderr, "  type:%d intval:%d strval:%s\n", st_variable[i].type, st_variable[i].u.int_value, st_variable[i].u.string_value);
  }

  fprintf(stderr, "========\n");


  return 0;
}

