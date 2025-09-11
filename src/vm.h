#include "errno.h"
#include <stdint.h>
#include <stdlib.h>

#define VM_STACK_SIZE 1024
#define VM_MAX_CALL_DEPTH 64
#define VM_MAX_LOCALS 256

typedef struct {
  int32_t locals[VM_MAX_LOCALS]; // Local variables
  size_t return_address;         // Return address for CALL/RET
  size_t prev_sp;                // Stack pointer
} VM_Frame;

typedef struct {
  uint8_t *code;     // Pointer to the bytecode instructions
  size_t code_size;  // Size of the bytecode instructions
  size_t ip;         // Instruction pointer
  int32_t *stack;    // Stack for the VM
  size_t stack_size; // Size of the stack
  size_t sp;         // Stack pointer
  VM_Frame call_stack[VM_MAX_CALL_DEPTH]; // Call stack for function calls
  size_t call_sp;                         // Call stack pointer
  size_t max_call_sp;                     // Maximum call stack depth
  ErrorCode error;                        // Error code for the last operation
} Nano_VM;

ErrorCode init_vm(Nano_VM *vm);
ErrorCode load_program(Nano_VM *vm, const uint8_t *code, size_t code_size,
                       uint32_t entry_point);
ErrorCode free_vm(Nano_VM *vm);
ErrorCode execute_vm(Nano_VM *vm);
