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
  int pc = 0;
  int sp = 0;

  while (pc < g_bytecode_size) {
    switch (g_bytecode[pc]) {
    case OP_PUSH_INT:
      st_stack[sp].type = INT_VALUE_TYPE;
      st_stack[sp].u.int_value
        = (int)g_bytecode[pc+1];
      sp++;
      pc += 2;
      break;
    case OP_PUSH_STRING:
      st_stack[sp].type = STRING_VALUE_TYPE;
      st_stack[sp].u.string_value
        = g_str_pool[g_bytecode[pc+1]];
      sp++;
      pc += 2;
      break;
    case OP_ADD:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        + st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_SUB:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        - st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_MUL:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        * st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_DIV:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        / st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_MINUS:
      st_stack[sp-1].u.int_value
        = -st_stack[sp-1].u.int_value;
      pc++;
      break;
    case OP_EQ:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        == st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_NE:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        != st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_GT:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        > st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_GE:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        >= st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_LT:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        < st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_LE:
      st_stack[sp-2].u.int_value
        = st_stack[sp-2].u.int_value
        <= st_stack[sp-1].u.int_value;
      sp--;
      pc++;
      break;
    case OP_PUSH_VAR:
      st_stack[sp]
        = st_variable[(int)g_bytecode[pc+1]];
      sp++;
      pc += 2;
      break;
    case OP_ASSIGN_TO_VAR:
      st_variable[(int)g_bytecode[pc+1]]
        = st_stack[sp-1];
      sp--;
      pc += 2;
      break;
    case OP_JUMP:
      pc = (int)g_bytecode[pc+1];
      break;
    case OP_JUMP_IF_ZERO:
      if (!st_stack[sp-1].u.int_value) {
        pc = (int)g_bytecode[pc+1];
      } else {
        pc += 2;
      }
      sp--;
      break;
    case OP_GOSUB:
      st_stack[sp].u.int_value = pc + 2;
      sp++;
      pc = (int)g_bytecode[pc+1];
      break;
    case OP_RETURN:
      pc = st_stack[sp-1].u.int_value;
      sp--;
      break;
    case OP_PRINT:
      if(st_stack[sp-1].type == INT_VALUE_TYPE){
        printf("%d\n", st_stack[sp-1].u.int_value);
      }else if(st_stack[sp-1].type == STRING_VALUE_TYPE){
        printf("%s\n", st_stack[sp-1].u.string_value);
      }
      sp--;
      pc++;
      break;
    default:
      assert(0);
    }
  }
}
