The instruction is defined as:
- 1- Input Address
  - (it defines where to read from: Memory or Input device)
- 2- Output Address
  - (it defines where to write: Memory or Output device)
- 3- Instruction for case 0
  - (executed when the bit in Path Selector address is 0)
- 4- Instruction for case 1
  - (executed when the bit in Path Selector address is 1)

Execution of one instruction: bit copy and path selection phases.
- BIT COPY
  - input phase (Read from input address)
  - output phase (write to output address a copy of the bit from input address)
- PATH SELECTION
  - instruction selection phase (instruction selection from Path Selector address,
    address that could be written or not in the output phase).

https://en.wikipedia.org/wiki/Microcode
https://en.wikipedia.org/wiki/One_instruction_set_computer
