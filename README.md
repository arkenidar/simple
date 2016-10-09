This project is about an architecture called "transport-triggered architechture" or similarly (the only instruction here is bit copying, or MOV as called by Wikipedia):
https://en.wikipedia.org/wiki/One_instruction_set_computer#Transport_triggered_architecture

It's a computing machinery usable to emulate other computing machinery defined in a RAM memory.
(#FPGA -like)
(simpler than a #RISC CPU)

This is the simplest computing machinery I can think of: the basic operation is composed of bit copy + path selection. It could be used to emulate/compute arbitrarily complex hardware/software system, even if the demos are quite basic (but they show the underlying principles!).

The instruction definition is made of:
- input address (it defines where to read from: memory or Input device)
- output address (it defines where to write: memory or Output device)
- instruction for case 0 (the bit in path chooser address is 0)
- instruction for case 1 (the bit in path chooser address is 1)

Phases of execution of 1 instruction:
 - input (Read from input address)
 - output (Write to output address a copy of the bit from input address, instruction selection from path chooser address (It could be written or not in the output phase. To write it the selected output address is the path chooser address))
 (Repeat in a loop to execute more than 1 instruction)
