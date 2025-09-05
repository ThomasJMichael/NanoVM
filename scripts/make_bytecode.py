import os

MAGIC = b'NBVM'
VERSION = 1
RESERVED = 0
CODE_SIZE = 1
ENTRY_POINT = 0

HALT_OPCODE = 19

output_dir = "test/data"
output_file = os.path.join(output_dir, "test_program.nvm")

os.makedirs(output_dir, exist_ok=True)

with open(output_file, "wb") as f:
    f.write(MAGIC)
    f.write(VERSION.to_bytes(2, 'little'))
    f.write(RESERVED.to_bytes(2, 'little'))
    f.write(CODE_SIZE.to_bytes(4, 'little'))
    f.write(ENTRY_POINT.to_bytes(4, 'little'))
    f.write(HALT_OPCODE.to_bytes(1, 'little'))

