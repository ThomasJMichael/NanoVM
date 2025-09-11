#ifndef BYTECODE_H
#define BYTECODE_H

#include <stddef.h>
#include <stdint.h>

#define MAX_OPERANDS 2 // The maximum number of operands for any instruction

typedef enum {
  // Data
  OP_PUSH,
  OP_POP,
  OP_LOAD,
  OP_STORE,
  OP_DUP,
  OP_SWAP,
  OP_OVER,
  OP_CLEAR,
  OP_PICK,

  // Arithmetic
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_INC,
  OP_DEC,

  // Comparison
  OP_CMP_EQ,
  OP_CMP_NEQ,
  OP_CMP_LT,
  OP_CMP_LTE,
  OP_CMP_GT,
  OP_CMP_GTE,

  // Control Flow
  OP_JMP,
  OP_JMPZ,
  OP_JMPNZ,
  OP_CALL,
  OP_RET,
  OP_NOP,

  // I/O
  OP_PRINT,
  OP_INPUT,

  // Halt
  OP_HALT

} Opcode;

typedef enum {
  OPERAND_NONE,
  OPERAND_IMMEDIATE,
  OPERAND_INDEX,
  OPERAND_ADDRESS,
  OPERAND_FLAG,
} OperandType;

typedef struct {
  const char *name;
  uint32_t length;
  uint8_t operand_count;
  OperandType operand_types[MAX_OPERANDS];
} InstructionInfo;

extern InstructionInfo instruction_set[];
#endif // BYTECODE_H
