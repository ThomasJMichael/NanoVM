#ifndef BYTECODE_FORMAT_H
#define BYTECODE_FORMAT_H

#include <stdint.h>

#define BYTECODE_MAGIC "\x4E\x42\x56\x4D" // "NBVM" in ASCII
#define BYTECODE_MAJOR_NUMER 0
#define BYTECODE_MINOR_NUMER 1
#define BYTECODE_VERSION ((BYTECODE_MAJOR_NUMER << 8) | (BYTECODE_MINOR_NUMER))

#define BYTECODE_MAGIC_OFFSET 0
#define BYTECODE_VERSION_OFFSET 4
#define BYTECODE_RESERVED_OFFSET 6
#define BYTECODE_CODE_SIZE_OFFSET 8
#define BYTECODE_ENTRY_POINT_OFFSET 12
#define BYTECODE_HEADER_SIZE 16

#define MAX_BYTECODE_SIZE (10 * 1024 * 1024) // 10 MB

typedef struct {
  char magic[4];        // Magic number to identify the bytecode format
  uint16_t version;     // Version of the bytecode format
  uint16_t reserved;    // Reserved for future use
  uint32_t code_size;   // Size of the bytecode instructions
  uint32_t entry_point; // Entry point of the bytecode
  // Additional fields can be added here (e.g., data segment size, entry point)
} BytecodeHeader;

#endif // BYTECODE_FORMAT_H
