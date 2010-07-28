#ifndef MINISCRIPT_H_INCLUDED
#define MINISCRIPT_H_INCLUDED

typedef enum {
  OP_PUSH_INT,
  OP_PUSH_STRING,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MINUS,
  OP_EQ,
  OP_NE,
  OP_GT,
  OP_GE,
  OP_LT,
  OP_LE,
  OP_PUSH_VAR,
  OP_ASSIGN_TO_VAR,
  OP_JUMP,
  OP_JUMP_IF_ZERO,
  OP_GOSUB,
  OP_RETURN,
  OP_PRINT
} OpCode; // オペコード

void mvm_execute(void);
#define VAR_MAX (4096)
extern int g_bytecode[];
extern int g_bytecode_size;
extern char *g_str_pool[];
extern int g_str_pool_size;

#endif
