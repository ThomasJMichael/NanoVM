
## General Rules
- Instructions: lower case only.
- Labels: case-sensitive, can be upper or lower case.
- Operands: separated by spaces.
- Comments: start with `;` (everything after is ignored).
- Whitespace: leading/trailing/empty lines ignored.
- File extension: `.asm`.

## Instruction Format
- Instructions appear at the start of a line or after a label.
- Example:  
  ```
  push 42
  add
  jmp LoopStart
  ```

## Labels
- Placed at the start of a line, followed by a colon (`:`).
- Case-sensitive: `LoopStart:` and `loopstart:` are different.
- Example:  
  ```
  LoopStart:
      push 1
      jmp LoopStart
  ```

## Comments
- Use `;` for comments.
- Example:  
  ```
  push 10 ; push value onto stack
  ```

## Immediate Values
- Numbers (decimal by default): `push 123`
- Hex (optional): `push 0x7B`

## Example Program
```
; countdown from 10 to 0
Start:
    push 10
Loop:
    dup
    jz End
    push 1
    sub
    jmp Loop
End:
    halt
