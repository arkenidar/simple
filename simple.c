#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h> // for memset
#include <stdbool.h>

#define SKIP_INPUT_REQUEST false

#define DEBUG_USING_PRINTF true
#define DEBUG2_USING_PRINTF false

#define INPUT_SYSTEM_WINDOWS 1
#define INPUT_SYSTEM_UNIX 2

#ifndef INPUT_SYSTEM

#ifndef _WIN32
#define INPUT_SYSTEM INPUT_SYSTEM_UNIX
#endif

#ifdef _WIN32
#define INPUT_SYSTEM INPUT_SYSTEM_WINDOWS
#endif

#endif

#if INPUT_SYSTEM == INPUT_SYSTEM_WINDOWS
#include <conio.h>
#endif

#define SIMPLE_BIT_ACCESS 1
#define BITWISE_OPS_BIT_ACCESS 2
#define BIT_ACCESS_IN_USE BITWISE_OPS_BIT_ACCESS

#if BIT_ACCESS_IN_USE == SIMPLE_BIT_ACCESS
	typedef uint8_t bit_type;
#endif
#if BIT_ACCESS_IN_USE == BITWISE_OPS_BIT_ACCESS
	typedef uint32_t bit_type;
#endif

bit_type memory[256] = {0};

// 8 bit data addressing (256 bits of data memory)
typedef char memory_index_type;

// 8 bit instruction addressing (256 instructions)
typedef char instruction_index_type;

typedef struct instruction_type_struct{
	memory_index_type mapping[2];
	instruction_index_type paths[2];
} instruction_type;

// reserved constants for memory address (e.g. mapping[0] or mapping[1])
#define PATHSEL 0
#define OUT 1
#define ZERO 2
#define ONE 3
#define IN 4

// reserved constants for instruction index (e.g. current_instruction_index, paths[0], paths[1], etc.)
#define PROGRAM_START_INDEX 0
instruction_index_type current_instruction_index = PROGRAM_START_INDEX;
#define END 0

// *********************************************************

// program: bit copy
instruction_type prog_bitcopy[] =	{
	{ {OUT,		IN},	{1,	1}	},
	{ {0,		0},	{-1,	-1}	}
};

// program: bit copy with end or repeat
instruction_type prog_bitcopy_end[] =	{
	{ {OUT,		IN},	{1,	1}	},
	{ {PATHSEL,	IN},	{-1,	END}	}
};

// program: NOT gate emulation
instruction_type prog_not[] =	{
	{ {PATHSEL,	IN},	{2,	1}	},
	{ {OUT,		ZERO},	{-1,	-1}	},
	{ {OUT,		ONE},	{-2,	-2}	}
};

// program: AND gate emulation
instruction_type prog_and[] =	{
	{ {PATHSEL,	IN},	{1,	4}	},
	{ {PATHSEL,	IN},	{1,	1}	},
	{ {OUT,		ZERO},	{-2,	-2}	},
	{ {OUT,		ONE},	{-3,	-3}	},
	{ {PATHSEL,	IN},	{-2,	-1}	}
};

// program: OR gate emulation
instruction_type prog_or[] =	{
	{ {PATHSEL,	IN},	{1,	4}	},
	{ {PATHSEL,	IN},	{1,	2}	},
	{ {OUT,		ZERO},	{-2,	-2}	},
	{ {OUT,		ONE},	{-3,	-3}	},
	{ {PATHSEL,	IN},	{-1,	-1}	}
};

// program: NAND gate emulation
instruction_type prog_nand[] =	{
	{ {PATHSEL,	IN},	{1,	4}	},
	{ {PATHSEL,	IN},	{1,	1}	},
	{ {OUT,		ONE},	{-2,	-2}	},
	{ {OUT,		ZERO},	{-3,	-3}	},
	{ {PATHSEL,	IN},	{-2,	-1}	}
};

// program: NOR gate emulation
instruction_type prog_nor[] =	{
	{ {PATHSEL,	IN},	{1,	4}	},
	{ {PATHSEL,	IN},	{1,	2}	},
	{ {OUT,		ONE},	{-2,	-2}	},
	{ {OUT,		ZERO},	{-3,	-3}	},
	{ {PATHSEL,	IN},	{-1,	-1}	}
};

// program: memory to output
instruction_type prog_memory_out[] =	{
	{ {10,		11},	{1,	1}	},
	{ {13,		14},	{1,	1}	},
	{ {OUT,		10},	{1,	1}	},
	{ {OUT,		11},	{1,	1}	},
	{ {OUT,		12},	{1,	1}	},
	{ {OUT,		13},	{1,	1}	},
	{ {OUT,		14},	{END,	END}	}
};

// program: output compare test
instruction_type prog_compare_test[] =	{
	{ {OUT,		ZERO},	{1,	1}	},
	{ {OUT,		ONE},	{1,	1}	},
	{ {OUT,		ZERO},	{END,	END}	}
};

// program: array
instruction_type prog_array[] =	{
	{ {5,		IN},	{1,	1} 	}, // 0) read x
	{ {6,		IN},	{1,	1} 	}, // 1) read i.0
	{ {7,		IN},	{1,	1} 	}, // 2) read i.1
	{ {PATHSEL,	6},	{1,	2} 	}, // 3) first level path separation
	{ {PATHSEL,	7},	{2,	3}	}, // 4) second level path separation
	{ {PATHSEL,	7},	{3,	4}	}, // 5) second level path separation
	{ {8,		5}, 	{4,	4}	}, // 6) a[0] = x (a[i]=x with i=0)
	{ {9,		5}, 	{3,	3}	}, // 7) a[1] = x (a[i]=x with i=1)
	{ {10,		5}, 	{2,	2}	}, // 8) a[2] = x (a[i]=x with i=2)
	{ {11,		5}, 	{1,	1}	}, // 9) a[3] = x (a[i]=x with i=3)
	{ {OUT,		8}, 	{1,	1}	}, // 10) print a[0]
	{ {OUT,		9}, 	{1,	1}	}, // 11) print a[1]
	{ {OUT,		10}, 	{1,	1}	}, // 12) print a[2]
	{ {OUT,		11}, 	{-13,	-13}	}, // 13) print a[3]
};

// *********************************************************

// use "program selector" to select which program to run in the Machine
instruction_type* g_prog_selector = prog_nor;

#if INPUT_SYSTEM == INPUT_SYSTEM_UNIX
#include <unistd.h>
#include <termios.h>
char getch(){
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=~ECHO;
    old.c_cc[VMIN]=1;
    old.c_cc[VTIME]=0;
    if(tcsetattr(0, TCSANOW, &old)<0)
        perror("tcsetattr ICANON");
    if(read(0,&buf,1)<0)
        perror("read()");
    old.c_lflag|=ICANON;
    old.c_lflag|=ECHO;
    if(tcsetattr(0, TCSADRAIN, &old)<0)
        perror ("tcsetattr ~ICANON");
    printf("%c",buf);
    return buf;
 }
#endif

#define INVALID_BIT -1
#define QUIT -2

#define PAUSE() get_char()

char get_char(){
	char ch;
	#if INPUT_SYSTEM == INPUT_SYSTEM_WINDOWS
		ch = _getche();
	#endif
	#if INPUT_SYSTEM == INPUT_SYSTEM_UNIX
		ch = getch();
	#endif
	return ch;
}

int getbit(){
	
	if(DEBUG_USING_PRINTF)
		printf(" in:");
	
	char ch = get_char();
	
	if (ch=='0' || ch=='1'){
		int bit = ch=='0'?0:1; // '0' or '1'
		if(DEBUG2_USING_PRINTF)
			printf("[getbit(): %d]", bit);
		return bit;
	}
	else if (ch=='q'){
		return QUIT;
	}
	else return INVALID_BIT;
}

#define BitVal(data,y)   ( (data>>y) & 1)  /** Return Data.Y value **/
#define ClearBit(data,y) data &= ~(1 << y) /** Clear Data.Y to 0   **/
#define SetBit(data,y)   data |= (1 << y)  /** Set Data.Y to 1     **/

#define bitArray_wordSize (sizeof(bit_type)*8)
#define bitArray_wordIndex(bit_address) (bit_address/bitArray_wordSize)
#define bitArray_bitIndex(bit_address)  (bit_address%bitArray_wordSize)

void write_bit_to_memory(memory_index_type write_to, int bit){

	#if BIT_ACCESS_IN_USE == SIMPLE_BIT_ACCESS
		memory[write_to] = bit;
	#endif

	#if BIT_ACCESS_IN_USE == BITWISE_OPS_BIT_ACCESS
		if(DEBUG2_USING_PRINTF)
			printf("[write_btm(): %d]", bit);
	
		int x = bitArray_wordIndex(write_to);
		int y = bitArray_bitIndex(write_to);
		
		if(bit==0)
			ClearBit(memory[x],y);
		else if(bit==1)
			SetBit(memory[x],y);
	#endif
}

int read_bit_from_memory(memory_index_type read_from){
	int value;
	#if BIT_ACCESS_IN_USE == SIMPLE_BIT_ACCESS
		value = memory[read_from];
	#endif

	#if BIT_ACCESS_IN_USE == BITWISE_OPS_BIT_ACCESS
		
		int x = memory[bitArray_wordIndex(read_from)];
		int y = bitArray_bitIndex(read_from);
		
		value = BitVal(x,y);
	#endif

	return value;
}

int read_bit_from_address(memory_index_type read_from){
	int value;
	if(read_from==IN) {
		if(true == SKIP_INPUT_REQUEST){
			//printf(" @skipped");
			return 0;
		}else
		while(true){
			value = getbit();
			if(value == INVALID_BIT){
				printf(" [insert bit (type '0' or '1') or quit (type 'q')!] ");
			} else if(value == QUIT){
				return QUIT;
			} else break;
		}
	} else if(read_from==ZERO) {
		value = 0;
	} else if(read_from==ONE) {
		value = 1;
	} else {
		value = read_bit_from_memory(read_from);
	}
	
	if(DEBUG2_USING_PRINTF)
		printf("[read_bfa(): %d]", value);
	
	return value;
}

int write_bit_to_address(memory_index_type write_to, int bit){
	write_bit_to_memory(write_to, bit);
	
	if(write_to==OUT){
		if(DEBUG_USING_PRINTF)
			printf(" out:%d", bit);
		return bit;
	}else{
		return INVALID_BIT;
	}
}

int perform_operation(instruction_type prog_selector[]){
	instruction_type instruction = prog_selector[(long long)current_instruction_index];
	int bit = read_bit_from_address(instruction.mapping[1]);
	if(bit==QUIT) return QUIT;
	else
		return write_bit_to_address(instruction.mapping[0], bit);
}

bool path_choice(instruction_type prog_selector[]){
	instruction_type instruction = prog_selector[(long long)current_instruction_index];
	int selector_bit = read_bit_from_memory(PATHSEL);
	int increment = instruction.paths[selector_bit];
	bool end;
	if(0 != increment){ current_instruction_index += increment; end=false; }
	else end=true;
	return end;
}

int test_bit_array(){
	int bits[] = {0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0};
	int size = sizeof(bits)/sizeof(int);
	int i;
	for(i = 0; i<size; i++)
		write_bit_to_memory(10+i, bits[i]);
	
	if(DEBUG2_USING_PRINTF)
		printf("\n");
	
	for(i = 0; i<size; i++)
		if(read_bit_from_memory(10+i)!=bits[i])
			return 0;
	return 1;	
}

#define STEPS_LIMIT_NO_LIMIT -1
#define STEPS_LIMIT STEPS_LIMIT_NO_LIMIT

int run_program(instruction_type* program_selector){
	const char* target_output = NULL; // can be set to NULL to disable
	
	int pause = false;
	
	printf(" { ");
	
	// counters for stats
	int cycle_counter = 0;
	int output_counter = 0;
	
	current_instruction_index = PROGRAM_START_INDEX;
	const char * target_output_cur = target_output;
	while(true){
	
		const int out = perform_operation(program_selector);
	
		if(out == QUIT){
			printf(" @QUIT");
			break;
		}
	
		if(out!=INVALID_BIT){
			
			// there is output
			
			if(target_output!=NULL){
				const char cur = *target_output_cur;
				
				const int cur_bit = cur-'0';
				assert(cur_bit==0 || cur_bit==1);
				
				if(out==cur_bit){
				
					output_counter++;
					target_output_cur++;
					if(*target_output_cur=='\0'){
						pause = true;
						break;
					}
				
				}else{
					break;
				}
			}else{
				output_counter++;			
			}
			
			if(false == DEBUG_USING_PRINTF)
				printf("%d", out);
		}
	
		bool end = path_choice(program_selector);
		if(end){
			printf(" @END");
			break;
		}
		
		cycle_counter++; if(STEPS_LIMIT!=STEPS_LIMIT_NO_LIMIT && cycle_counter>=STEPS_LIMIT) break;
	}
	
	printf(" } \n");
	
	if(pause == true) PAUSE();
	
	return output_counter;
}

void bootstrap_tests(){
	if(DEBUG2_USING_PRINTF)
		printf("- bitArray_wordSize: %d\n", (int)bitArray_wordSize);
	
	int bit_array_works = test_bit_array();
	assert(bit_array_works); 
	
	if(DEBUG2_USING_PRINTF)
		printf("- test_bit_array(): %d\n", bit_array_works);
}

void multiple_programs_executed_sequentially(){
	run_program(prog_bitcopy);
	run_program(prog_not);
	run_program(prog_memory_out);
}

#define MODULO_MAX 4//16
#define MODULO_INCREMENT(expr, max)  expr=(expr+1)%max;

#define FIELD_1 mapping[0]
#define FIELD_2 mapping[1]
#define FIELD_3 paths[0]
#define FIELD_4 paths[1]

void print_instruction(instruction_type* instruction){
	printf("(%d,%d,%d,%d) ",
	instruction->FIELD_1,
	instruction->FIELD_2,
	instruction->FIELD_3,
	instruction->FIELD_4);
}

void print_program(instruction_type program[], const int size){
	for(int i=0; i<size; i++)
		print_instruction(&program[i]);
	printf("\n");
}

int increment_instruction(instruction_type* instruction){
	
	MODULO_INCREMENT(instruction->FIELD_1, MODULO_MAX)
	if(0 == instruction->FIELD_1) {
			
		MODULO_INCREMENT(instruction->FIELD_2, MODULO_MAX)
		if(0 == instruction->FIELD_2){
			
			MODULO_INCREMENT(instruction->FIELD_3, MODULO_MAX)
			if(0 == instruction->FIELD_3) {
				
				MODULO_INCREMENT(instruction->FIELD_4, MODULO_MAX)
				if(0 == instruction->FIELD_4){

					return true; // overflow, full iteration cycle
				}
			}

		}	
	}
	return false;
}

int next_program(instruction_type program[], const int size){

	int instruction_index = 0; // instruction index
	int overflow;
	
	while(true){
		overflow = increment_instruction(&program[instruction_index]);
		
		if(overflow){
			instruction_index++;

			if(size <= instruction_index){
				overflow = true; // full iteration completed
				break;
			}
		}else{
			overflow = false; // full iteration not completed
			break;
		}
	}
	return overflow;
}

void reset_memory(){
	const int memory_size = sizeof(memory)/sizeof(bit_type);
	// set all memory to zero
	memset(memory, 0, memory_size);
}

int iterate_programs(){
	
	// program
	instruction_type* current_program;
	const int PROGRAM_SIZE = 4;
	current_program = (instruction_type*) malloc(sizeof(instruction_type)*PROGRAM_SIZE);

	// stats
	int count_programs = 0;
	int max_oc = -1;
	
	while(true){ // iterate programs
		
		// - run program
		
		print_program(current_program, PROGRAM_SIZE);
		
		reset_memory();
		int output_counter = run_program(current_program);
		
		// output stats
		printf(" (OC:%d)\n", output_counter);
				
		if(output_counter>max_oc){
			max_oc = output_counter;
		}
		
		count_programs++; // count programs

		if(true == next_program(current_program, PROGRAM_SIZE)){
			break; // iteration completed
		}
	}
	
	printf("(count_programs:%d)", count_programs);
	
	return max_oc;
}

int main(int argc, char **argv) {
	bootstrap_tests();
	
	//run_program(g_prog_selector);
	multiple_programs_executed_sequentially();
	return 0;
}
