
# NanoVM

NanoVM is a lightweight virtual machine for executing custom bytecode programs.  
It features modular logging, bytecode loading, and a simple command-line interface.

## Features

- Loads and executes bytecode files
- Configurable logging (stderr or file)
- Command-line options for bytecode and log file selection
- Unit testing with Unity framework

## Requirements

- GCC (or compatible C compiler)
- GNU Make

## Building

To build NanoVM:

```sh
make
```

For debug, trace, or release builds:

```sh
make debug
make trace
make release
```

## Running

```sh
./nanovm -f <bytecode_file> [-l <log_file>]
```

## Testing

To build and run all unit tests:

```sh
make test
```

## Cleaning

To remove build artifacts:

```sh
make clean
```

## License

See [LICENSE](LICENSE) for details.
