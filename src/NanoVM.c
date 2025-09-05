#include "loader.h"
#include "log.h"
#include "vm.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

ErrorCode init_logging(const char *log_file_path) {
  ErrorCode status = SUCCESS;

  log_set_level(LOG_LEVEL);

  if (log_file_path != NULL) {
    FILE *fp = fopen(log_file_path, "a");
    if (!fp || log_add_fp(fp, LOG_LEVEL) == -1) {
      log_error("Failed to open log file: %s", log_file_path);
      status = ERR_FILE_NOT_FOUND;
    }
  } else {
    if (log_add_fp(stderr, LOG_LEVEL) == -1) {
      log_error("Failed to set up stderr logging");
      status = ERR_UNKNOWN;
    }
  }

  log_info("Logging initialized");
  return status;
}

ErrorCode parse_args(int argc, char *argv[], char **bytecode_file,
                     char **log_file_path) {
  if (argc < 1) {
    log_error("No arguments provided.");
    return ERR_INVALID_OPERAND;
  }

  // Example: Just log the arguments for now
  // TODO: Remove this logging in production
  for (int i = 0; i < argc; i++) {
    log_info("Arg %d: %s", i, argv[i]);
  }

  int opts;
  while ((opts = getopt(argc, argv, "hfl:")) != -1) {
    switch (opts) {
    case 'h':
      printf("Usage: %s [options] <bytecode_file>\n", argv[0]);
      printf("Options:\n");
      printf("  -h                Show this help message\n");
      printf("  -f <file>         Specify the bytecode file to load\n");
      printf("  -l <file>  Output logs to specified file\n");
      return SUCCESS;
    case 'f':
      log_info("Bytecode file specified: %s", optarg);
      *bytecode_file = optarg;
      break;
    case 'l':
      log_info("Log file specified: %s", optarg);
      *log_file_path = optarg;
      break;
    case '?':
    default:
      log_error("Unknown option: %c", optopt);
      return ERR_INVALID_OPERAND;
    }
  }
  return SUCCESS;
}

int main(int argc, char *argv[]) {
  ErrorCode status = SUCCESS;
  char *log_file_path = NULL;
  char *bytecode_file = NULL;
  Nano_VM vm;
  uint32_t entry_point;
  size_t size;
  uint8_t *bytecode_buffer = NULL;

  status = parse_args(argc, argv, &bytecode_file, &log_file_path);
  if (status != SUCCESS) {
    log_error("Failed to parse arguments");
    return status;
  }
  status = init_logging(log_file_path);
  if (status != SUCCESS) {
    log_error("Failed to initialize logging");
    return status;
  }
  status = init_vm(&vm);
  if (status != SUCCESS) {
    log_error("Failed to initialize VM");
    goto CLEANUP;
  }
  status = load_bytecode(bytecode_file, &bytecode_buffer, &size, &entry_point);
  if (status != SUCCESS) {
    log_error("Failed to load bytecode");
    goto CLEANUP;
  }
  status = load_program(&vm, bytecode_buffer, size);
  if (status != SUCCESS) {
    log_error("Failed to load program into VM");
    goto CLEANUP;
  }
  status = execute_vm(&vm);
  if (status != SUCCESS) {
    log_error("VM execution failed with error code: %d", status);
    goto CLEANUP;
  }
  status = free_bytecode(&bytecode_buffer);
  if (status != SUCCESS) {
    log_error("Failed to free bytecode buffer");
    goto CLEANUP;
  }
  status = free_vm(&vm);
  if (status != SUCCESS) {
    log_error("Failed to free VM resources");
    goto CLEANUP;
  }
CLEANUP:
  if (bytecode_buffer) {
    free_bytecode(&bytecode_buffer);
  }
  if (vm.code || vm.stack) {
    free_vm(&vm);
  }
  return status;
}
