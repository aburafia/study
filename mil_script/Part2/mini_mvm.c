#include <stdio.h>
#include <assert.h>

#define STACK_SIZE_MAX (65536)

typedef enum {
  OP_PUSH_INT,
  OP_ADD,
  OP_MUL,
  OP_PRINT
} OpCode;

int g_bytecode[] = {
  (int)OP_PUSH_INT,
       10,
  (int)OP_PUSH_INT,
       2,
  (int)OP_PUSH_INT,
       4,
  (int)OP_MUL,
  (int)OP_ADD,
  (int)OP_PRINT,
};

int st_stack[STACK_SIZE_MAX];

void mvm_execute(void){
  int pc = 0;
  int sp = 0; // スタックポインタ

  while (pc < sizeof(g_bytecode) / sizeof(*g_bytecode)) {
    switch (g_bytecode[pc]) {
    case OP_PUSH_INT: // 整数をスタックに積む
      st_stack[sp] = (int)g_bytecode[pc+1];
      sp++;
      pc += 2;
      break;
    case OP_ADD: // 加算
      st_stack[sp-2] = st_stack[sp-2] + st_stack[sp-1];
      sp--;
      pc++;
      break;
    case OP_MUL: // 乗算
      st_stack[sp-2] = st_stack[sp-2] * st_stack[sp-1];
      sp--;
      pc++;
      break;
    case OP_PRINT: // 表示
      printf("%d\n", st_stack[sp-1]);
      sp--;
      pc++;
      break;
    default:
      assert(0);
    }
  }
}

int main(void){

  mvm_execute();

  return 0;
}
