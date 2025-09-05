#include "vm.h"
#include "bytecode.h"
#include "bytecode_format.h"
#include "errno.h"
#include "log.h"
#include <stdint.h>
#include <string.h>

ErrorCode init_vm(Nano_VM *vm) {
  ErrorCode status = SUCCESS;
  vm->stack = malloc(sizeof(int32_t) * VM_STACK_SIZE);
  if (NULL == vm->stack) {
    log_error("Failed to allocate memory for VM stack.");
    return ERR_OUT_OF_MEMORY;
  }

  vm->stack_size = VM_STACK_SIZE;
  vm->sp = 0;

  vm->call_sp = 1;
  memset(&vm->call_stack, 0, sizeof(VM_Frame));
  vm->call_stack[0].prev_sp = 0;
  vm->call_stack[0].return_address = 0;
  vm->max_call_sp = 0;

  vm->code = NULL;
  vm->code_size = 0;
  vm->ip = 0;
  vm->error = SUCCESS;

  return status;
}

ErrorCode load_program(Nano_VM *vm, const uint8_t *code, size_t code_size) {
  ErrorCode status = SUCCESS;
  if (NULL == vm) {
    log_error("VM instance is NULL");
    return ERR_NULL_POINTER;
  }

  if (NULL == code || code_size == 0) {
    log_error("Invalid code or code size");
    return ERR_INVALID_OPERAND;
  }

  if (NULL != vm->code) {
    free(vm->code);
    vm->code = NULL;
    vm->code_size = 0;
    log_warn("Existing bytecode in VM was overwritten");
  }

  vm->code = malloc(code_size);
  if (NULL == vm->code) {
    log_error("Failed to allocate memory for bytecode");
    return ERR_OUT_OF_MEMORY;
  }

  vm->code_size = code_size;
  memcpy(vm->code, code, code_size);
  log_info("Bytecode loaded into VM (%zu bytes)", code_size);

  size_t entry_point;
  memcpy(&entry_point, code + BYTECODE_ENTRY_POINT_OFFSET, sizeof(uint32_t));
  vm->ip = entry_point;
  log_info("Entry point set to: %zu", entry_point);
  return status;
}

ErrorCode free_vm(Nano_VM *vm) {
  ErrorCode status = SUCCESS;
  if (NULL == vm) {
    log_error("VM instance is NULL");
    return ERR_NULL_POINTER;
  }

  if (NULL != vm->code) {
    free(vm->code);
    vm->code = NULL;
    vm->code_size = 0;
  }

  if (NULL != vm->stack) {
    free(vm->stack);
    vm->stack = NULL;
    vm->stack_size = 0;
    vm->sp = 0;
  }

  log_info("VM resources freed");
  return status;
}

/* Instruction dispatch loop
 * 1. Fetch the next instruction using the instruction pointer (ip).
 * 2. Decode the instruction to determine the opcode and operands.
 * 3. Execute the instruction by performing the operation specified by the
 * opcode.
 * 4. Update the instruction pointer (ip) to point to the next instruction.
 * 5. Handle errors such as stack overflow, stack underflow, invalid opcode,
 * etc.
 * 6. Repeat until a HALT instruction is encountered or an error occurs.
 */
ErrorCode execute_vm(Nano_VM *vm) {
  ErrorCode status = SUCCESS;
  if (NULL == vm) {
    log_error("VM instance is NULL");
    return ERR_NULL_POINTER;
  }

  if (vm->code == NULL || vm->code_size == 0) {
    log_error("No bytecode loaded in VM");
    return ERR_INVALID_OPERAND;
  }

  if (vm->ip >= vm->code_size) {
    log_error("Instruction pointer out of bounds: %zu", vm->ip);
    return ERR_INVALID_OPERAND;
  }

  while (1) {
    log_debug("IP: %zu, SP: %zu", vm->ip, vm->sp);
    Opcode opcode = vm->code[vm->ip];
    InstructionInfo info = instruction_set[opcode];

    switch (opcode) {
    case OP_PUSH: {
      if (vm->ip + info.length >= vm->code_size) {
        log_error("PUSH instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp >= vm->stack_size) {
        log_error("Stack overflow on PUSH");
        status = ERR_STACK_OVERFLOW;
        goto VM_EXIT;
      }
      int32_t value;
      memcpy(&value, vm->code + vm->ip + 1, sizeof(int32_t));
      vm->stack[vm->sp++] = value;
      vm->ip += info.length;
      break;
    }
    case OP_POP: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("POP instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp == 0) {
        log_error("Stack underflow on POP");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      int32_t value;
      memcpy(&value, &vm->stack[--vm->sp], sizeof(int32_t));
      vm->ip += info.length;
      break;
    }
    case OP_LOAD: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("LOAD instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      uint32_t index = vm->code[vm->ip + 1];
      if (index >= VM_MAX_LOCALS) {
        log_error("Local variable index out of bounds: %u", index);
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp >= vm->stack_size) {
        log_error("LOAD instruction stack overflow");
        status = ERR_STACK_OVERFLOW;
        goto VM_EXIT;
      }
      vm->stack[vm->sp++] =
          vm->call_stack[vm->call_sp - 1].locals[vm->code[vm->ip + 1]];
      vm->ip += info.length;
      break;
    }
    case OP_STORE: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("STORE instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      uint32_t index = vm->code[vm->ip + 1];
      if (index >= VM_MAX_LOCALS) {
        log_error("Local variable index out of bounds: %u", index);
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp == 0) {
        log_error("Stack underflow on STORE");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      vm->call_stack[vm->call_sp - 1].locals[index] = vm->stack[--vm->sp];
      vm->ip += info.length;
      break;
    }
    case OP_ADD: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("ADD instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on ADD");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = a + b;
      vm->ip += info.length;
      break;
    }
    case OP_SUB: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("SUB instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on SUB");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = a - b;
      vm->ip += info.length;
      break;
    }
    case OP_MUL: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("MUL instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on MUL");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = a * b;
      vm->ip += info.length;
      break;
    }
    case OP_DIV: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("DIV instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on DIV");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      if (b == 0) {
        log_error("Division by zero");
        status = ERR_DIVIDE_BY_ZERO;
        goto VM_EXIT;
      }
      vm->stack[vm->sp++] = a / b;
      vm->ip += info.length;
      break;
    }
    case OP_CMP_EQ: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("CMP_EQ instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on CMP_EQ");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = (a == b) ? 1 : 0;
      vm->ip += info.length;
      break;
    }
    case OP_CMP_NEQ: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("CMP_NEQ instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on CMP_NEQ");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = (a != b) ? 1 : 0;
      vm->ip += info.length;
      break;
    }
    case OP_CMP_LT: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("CMP_LT instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on CMP_LT");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = (a < b) ? 1 : 0;
      vm->ip += info.length;
      break;
    }
    case OP_CMP_LTE: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("CMP_LTE instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on CMP_LTE");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = (a <= b) ? 1 : 0;
      vm->ip += info.length;
      break;
    }
    case OP_CMP_GT: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("CMP_GT instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on CMP_GT");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = (a > b) ? 1 : 0;
      vm->ip += info.length;
      break;
    }
    case OP_CMP_GTE: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("CMP_GTE instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp < 2) {
        log_error("Stack underflow on CMP_GTE");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      uint32_t b = vm->stack[--vm->sp];
      uint32_t a = vm->stack[--vm->sp];
      vm->stack[vm->sp++] = (a >= b) ? 1 : 0;
      vm->ip += info.length;
      break;
    }
    case OP_JMP: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("JMP instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }

      if (vm->ip + 1 >= vm->code_size) {
        log_error("JMP destination out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }

      vm->ip = *((uint32_t *)(vm->code + vm->ip + 1));
      break;
    }
    case OP_JMPZ: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("JMPZ instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }

      if (vm->sp < 1) {
        log_error("Stack underflow on JMPZ");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }

      uint32_t stack_value = vm->stack[--vm->sp];
      if (stack_value == 0) {
        vm->ip = *((uint32_t *)(vm->code + vm->ip + 1));
      } else {
        vm->ip += info.length;
      }
      break;
    }
    case OP_CALL: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("CALL instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }

      if (vm->call_sp >= VM_MAX_CALL_DEPTH) {
        log_error("Call stack overflow on CALL");
        status = ERR_STACK_OVERFLOW;
        goto VM_EXIT;
      }

      VM_Frame *new_frame = &vm->call_stack[vm->call_sp++];
      new_frame->return_address = vm->ip + info.length;
      new_frame->prev_sp = vm->sp;
      vm->ip = *((uint32_t *)(vm->code + vm->ip + 1));
      break;
    }
    case OP_RET: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("RET instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->call_sp <= 1) {
        log_error("Call stack underflow on RET");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      vm->sp = vm->call_stack[--vm->call_sp].prev_sp;
      vm->ip = vm->call_stack[vm->call_sp].return_address;
      break;
    }
    case OP_PRINT: {
      if (vm->ip + info.length > vm->code_size) {
        log_error("PRINT instruction out of bounds");
        status = ERR_INVALID_OPERAND;
        goto VM_EXIT;
      }
      if (vm->sp == 0) {
        log_error("Stack underflow on PRINT");
        status = ERR_STACK_UNDERFLOW;
        goto VM_EXIT;
      }
      int32_t value = vm->stack[--vm->sp];
      printf("%d\n", value);
      vm->ip += info.length;
      break;
    }
    case OP_HALT: {
      status = SUCCESS;
      log_info("HALT instruction encountered. Stopping execution.");
      goto VM_EXIT;
    }
    }
  }
VM_EXIT:
  vm->error = status;
  log_info("VM execution ended with status: %d", status);
  return status;
}
