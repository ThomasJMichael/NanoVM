
#include "errno.h"
#include "loader.h"
#include "unity.h"
#include "unity_internals.h"
#include "vm.h"

void setUp(void) {}
void tearDown(void) {}

void test_init_vm_success(void) {
  Nano_VM vm;
  ErrorCode err = init_vm(&vm);
  TEST_ASSERT_EQUAL_INT(SUCCESS, err);
}

void test_load_program_invalid_args(void) {
  Nano_VM vm;

  ErrorCode err = load_program(&vm, NULL, 0, 0);
  TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERAND, err);
}

void test_execute_vm_halt(void) {
  const char *bytecode_file = "test/data/test_program.nvm";
  uint8_t *bytecode_buffer = NULL;
  size_t size = 0;
  uint32_t entry_point = 0;

  ErrorCode status;
  Nano_VM vm;
  status = init_vm(&vm);
  TEST_ASSERT_EQUAL_INT(SUCCESS, status);
  status = load_bytecode(bytecode_file, &bytecode_buffer, &size, &entry_point);
  TEST_ASSERT_EQUAL_INT(SUCCESS, status);
  status = load_program(&vm, bytecode_buffer, size, entry_point);
  TEST_ASSERT_EQUAL_INT(SUCCESS, status);
  status = execute_vm(&vm);
  TEST_ASSERT_EQUAL_INT(SUCCESS, status);
  status = free_bytecode(&bytecode_buffer);
  TEST_ASSERT_EQUAL_INT(SUCCESS, status);
}

void test_free_vm_null(void) {
  ErrorCode err = free_vm(NULL);
  TEST_ASSERT_EQUAL_INT(ERR_NULL_POINTER, err);
}

int main(void) {
  RUN_TEST(test_init_vm_success);
  RUN_TEST(test_load_program_invalid_args);
  RUN_TEST(test_execute_vm_halt);
  RUN_TEST(test_free_vm_null);
}
