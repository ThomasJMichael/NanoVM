#include "loader.h"
#include "bytecode_format.h"
#include "errno.h"
#include "log.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

ErrorCode load_bytecode(const char *filename, uint8_t **code_buffer,
                        size_t *code_size, uint32_t *entry_point) {
  FILE *file;
  ErrorCode status = SUCCESS;
  uint8_t *full_buffer = NULL;
  size_t file_size = 0;

  file = fopen(filename, "rb");
  if (NULL == file) {
    log_error("Failed to open bytecode file: %s", filename);
    return ERR_FILE_NOT_FOUND;
  }

  fseek(file, 0, SEEK_END);
  file_size = (size_t)ftell(file);
  fseek(file, 0, SEEK_SET);
  log_debug("Bytecode file size: %zu bytes", file_size);

  if (file_size < BYTECODE_HEADER_SIZE) {
    fclose(file);
    log_error("Bytecode file size is too small: %zu bytes", file_size);
    return ERR_INVALID_FORMAT;
  }
  if (file_size > MAX_BYTECODE_SIZE) {
    fclose(file);
    log_error("Bytecode file size is too large: %zu bytes", file_size);
    return ERR_FILE_TOO_LARGE;
  }

  full_buffer = (uint8_t *)malloc(file_size);
  if (NULL == full_buffer) {
    fclose(file);
    log_error("Failed to allocate memory for bytecode buffer");
    return ERR_OUT_OF_MEMORY;
  }

  if (fread(full_buffer, 1, file_size, file) != file_size) {
    free(full_buffer);
    fclose(file);
    log_error("Failed to read bytecode file: %s", filename);
    return ERR_FILE_READ;
  }
  fclose(file);

  status = verify_bytecode_format(full_buffer, file_size);
  if (status != SUCCESS) {
    free(full_buffer);
    log_error("Bytecode format verification failed");
    return status;
  }

  uint32_t code_sz;
  memcpy(&code_sz, full_buffer + BYTECODE_CODE_SIZE_OFFSET, sizeof(uint32_t));
  memcpy(entry_point, full_buffer + BYTECODE_ENTRY_POINT_OFFSET,
         sizeof(uint32_t));

  if (code_sz + BYTECODE_HEADER_SIZE > file_size) {
    free(full_buffer);
    log_error("Declared code size exceeds file size: %u > %zu",
              code_sz + BYTECODE_HEADER_SIZE, file_size);
    return ERR_INVALID_FORMAT;
  }

  *code_buffer = (uint8_t *)malloc(code_sz);
  if (NULL == *code_buffer) {
    free(full_buffer);
    log_error("Failed to allocate memory for code segment");
    return ERR_OUT_OF_MEMORY;
  }

  memcpy(*code_buffer, full_buffer + BYTECODE_HEADER_SIZE, code_sz);
  *code_size = code_sz;

  free(full_buffer);

  log_info("Bytecode file '%s' loaded successfully (code size: %zu bytes, "
           "entry point: %u)",
           filename, *code_size, *entry_point);
  return SUCCESS;
}

ErrorCode verify_bytecode_format(const uint8_t *buffer, size_t size) {
  if (size < BYTECODE_HEADER_SIZE) {
    log_error("Bytecode file too small to contain valid header");
    return ERR_INVALID_FORMAT;
  }

  uint8_t magic[4];
  memcpy(&magic, buffer + BYTECODE_MAGIC_OFFSET, 4);
  if (memcmp(magic, BYTECODE_MAGIC, 4) != 0) {
    log_error("Invalid magic number: 0x%X expected: 0x%X", magic,
              BYTECODE_MAGIC);
    return ERR_INVALID_FORMAT;
  }

  uint16_t version;
  memcpy(&version, buffer + BYTECODE_VERSION_OFFSET, sizeof(uint16_t));
  if (version != BYTECODE_VERSION) {
    log_error("Unsupported bytecode version: 0x%04X\nCurrent version: 0x%04X",
              version, BYTECODE_VERSION);
    return ERR_INVALID_FORMAT;
  }

  uint32_t code_size;
  memcpy(&code_size, buffer + BYTECODE_CODE_SIZE_OFFSET, sizeof(uint32_t));
  if (code_size + 12 > size) { // 12 bytes for header
    log_error("Declared code size exceeds file size: %u > %zu", code_size + 12,
              size);
    return ERR_INVALID_FORMAT;
  }

  uint32_t entry_point;
  memcpy(&entry_point, buffer + BYTECODE_ENTRY_POINT_OFFSET, sizeof(uint32_t));
  if (entry_point >= code_size) {
    log_error("Invalid entry point: %u", entry_point);
    return ERR_INVALID_FORMAT;
  }

  log_info(
      "Bytecode format verified: version 0x%04X, code size %u, entry point %u",
      version, code_size, entry_point);
  return SUCCESS;
}

ErrorCode free_bytecode(uint8_t **buffer) {
  if (NULL != *buffer) {
    free(*buffer);
    *buffer = NULL;
    log_info("Bytecode buffer freed");
    return SUCCESS;
  }
  log_warn("Attempted to free a NULL bytecode buffer");
  return ERR_NULL_POINTER;
}
