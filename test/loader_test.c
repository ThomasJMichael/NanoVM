#include "bytecode_format.h"
#include "errno.h"
#include "loader.h"
#include "unity.h"
#include <stdlib.h>

static const char *filename;
static uint8_t *buffer = NULL;
static size_t size = 0;
static uint32_t entry_point = 0;

void reset_loader_test_state(void) {
  filename = NULL;
  if (buffer) {
    free(buffer);
    buffer = NULL;
  }
  size = 0;
  entry_point = 0;
}

void setUp(void) {
  // Runs before each test
  filename = "test/data/test_program.nvm";
  buffer = NULL;
  size = 0;
}

void tearDown(void) {
  // Runs after each test
  if (buffer) {
    free(buffer);
    buffer = NULL;
  }
}

void test_load_bytecode_valid_file(void) {
  int result = load_bytecode(filename, &buffer, &size, &entry_point);
  TEST_ASSERT_EQUAL_INT(SUCCESS, result);
  TEST_ASSERT_NOT_NULL(buffer);
  TEST_ASSERT(size > 0);
  reset_loader_test_state();
}

void test_load_invalid_file_path(void) {
  filename = "non_existent_file.nvm";
  ErrorCode result = load_bytecode(filename, &buffer, &size, &entry_point);
  TEST_ASSERT_EQUAL_INT(ERR_FILE_NOT_FOUND, result);
  reset_loader_test_state();
}
void test_load_file_too_small(void) {
  filename = "too_small.nvm";
  FILE *fp = fopen(filename, "wb");
  fwrite("NB", 1, 2, fp); // Write only 2 bytes
  fclose(fp);
  ErrorCode result = load_bytecode(filename, &buffer, &size, &entry_point);
  TEST_ASSERT_EQUAL_INT(ERR_INVALID_FORMAT, result);
  remove(filename);
}
void test_load_file_too_large(void) {
  // TODO: Make sure to gitignore this test file
  filename = "test/data/too_large.nvm";
  FILE *f = fopen(filename, "rb");
  if (!f) {
    // Create file only if it doesn't exist
    f = fopen(filename, "wb");
    fseek(f, MAX_BYTECODE_SIZE, SEEK_SET);
    fputc(0, f); // Write a single byte at the end
    fclose(f);
  } else {
    fclose(f);
  }

  ErrorCode result = load_bytecode(filename, &buffer, &size, &entry_point);
  TEST_ASSERT_EQUAL_INT(ERR_FILE_TOO_LARGE, result);

  reset_loader_test_state();
}
void test_load_code_size_exceeds_file(void) {
  filename = "code_size_exceeds.nvm";
  FILE *fp = fopen(filename, "wb");
  // Write a valid header with code_size = 100, but only 10 bytes of code
  uint8_t header[8] = {'N', 'B', 0, 0,
                       0,   100, 0, 0}; // Example header, adjust as needed
  fwrite(header, 1, 8, fp);
  uint8_t code[10] = {0};
  fwrite(code, 1, 10, fp);
  fclose(fp);

  ErrorCode result = load_bytecode(filename, &buffer, &size, &entry_point);
  TEST_ASSERT_EQUAL_INT(ERR_INVALID_FORMAT, result);

  remove(filename);
}

void test_free_bytecode_buffer(void) {
  uint8_t *test_buffer = malloc(16);
  TEST_ASSERT_NOT_NULL(test_buffer);
  free_bytecode(&test_buffer);
  TEST_ASSERT_NULL(test_buffer);
}
// TODO: Implement rest
void test_load_entry_point_out_of_bounds(void);
void test_load_out_of_memory(void);
void test_multiple_loads(void);
void test_load_corrupted_code_segment(void);

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_load_bytecode_valid_file);
  RUN_TEST(test_load_invalid_file_path);
  RUN_TEST(test_load_file_too_small);
  RUN_TEST(test_load_file_too_large);
  RUN_TEST(test_load_code_size_exceeds_file);
  RUN_TEST(test_free_bytecode_buffer);
  return UNITY_END();
}
