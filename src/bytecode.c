#include "bytecode.h"

InstructionInfo instruction_set[] = {
    // Data
    {"PUSH", 2, 1, {OPERAND_IMMEDIATE}},
    {"POP", 1, 0, {OPERAND_NONE}},
    {"LOAD", 2, 1, {OPERAND_INDEX}},
    {"STORE", 2, 1, {OPERAND_INDEX}},

    // Arithmetic
    {"ADD", 1, 0, {OPERAND_NONE}},
    {"SUB", 1, 0, {OPERAND_NONE}},
    {"MUL", 1, 0, {OPERAND_NONE}},
    {"DIV", 1, 0, {OPERAND_NONE}},

    // Comparison
    {"CMP_EQ", 1, 0, {OPERAND_NONE}},
    {"CMP_NEQ", 1, 0, {OPERAND_NONE}},
    {"CMP_LT", 1, 0, {OPERAND_NONE}},
    {"CMP_LTE", 1, 0, {OPERAND_NONE}},
    {"CMP_GT", 1, 0, {OPERAND_NONE}},
    {"CMP_GTE", 1, 0, {OPERAND_NONE}},

    // Control Flow
    {"JMP", 2, 1, {OPERAND_ADDRESS}},
    {"JMPZ", 2, 1, {OPERAND_ADDRESS}},
    {"CALL", 2, 1, {OPERAND_ADDRESS}},
    {"RET", 1, 0, {OPERAND_NONE}},

    // Output
    {"PRINT", 1, 0, {OPERAND_NONE}},

    // Halt
    {"HALT", 1, 0, {OPERAND_NONE}},

    // Sentinel to mark the end of the array
    {NULL, 0, 0, {OPERAND_NONE}}};
