this is a "transport-triggered architechture" or similar (only instruction is bit copying, or MOV)
https://en.wikipedia.org/wiki/One_instruction_set_computer#Transport_triggered_architecture

computing machinery usable to emulate other computing machinery defined in a RAM memory. (#FPGA -like) (simpler than a #RISC CPU)

this is the simplest computing machinery I can think of: basic operation is composed of bit copy + path selection. it could be used to emulate/compute arbitrarily complex hardware/software system, even if demos are quite basic (but they show the underlying principles!)

instruction definition is made of:
- input address
- output address
- instruction for case 0
- instruction for case 1

phases of execution of 1 instruction:
 - input (read from input address)
 - output (write to output address a copy of the bit from input address, instruction selection from path chooser address (it could be written or not in the output phase. to write it output address is the path chooser address))
 (repeat in a loop to execute more than 1 instruction)
