/* 1. Open bytecode file
 * 2. Read contents into buffer
 * 3. Verify the format (e.g., magic number, version)
 * 4. Handle errors (file not found, read errors, invalid format)
 * 5. Prepare the bytecode for execution in vm loop
 */

#include "errno.h"
#include <stdint.h>
#include <stdio.h>

/* Loads bytecode from a file into a buffer.
 * Parameters:
 *  filename - Path to the bytecode file
 *  code_buffer - Pointer to the buffer where bytecode will be stored
 *  code_size - Pointer to size variable where the size of the bytecode will be
 *  entry_point - Pointer to variable where the entry point will be
 * stored Returns: ErrorCode indicating success or type of failure
 */
ErrorCode load_bytecode(const char *filename, uint8_t **code_buffer,
                        size_t *code_size, uint32_t *entry_point);

/* Verifies the format of the loaded bytecode.
 * Parameters:
 *   buffer - Pointer to the buffer containing the bytecode
 *   size - Size of the bytecode in bytes
 * Returns:
 *   ErrorCode indicating success or type of failure
 */
ErrorCode verify_bytecode_format(const uint8_t *buffer, size_t size);

/* Frees the allocated bytecode buffer.
 * Parameters:
 *   buffer - Pointer to the buffer to be freed
 * Returns:
 *   ErrorCode indicating success or type of failure
 */
ErrorCode free_bytecode(uint8_t **buffer);
