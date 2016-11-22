#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h> // for memset

const int FALSE = (0==1);
const int TRUE  = (0==0);
#define SKIP_INPUT_REQUEST TRUE

#define DEBUG_USING_PRINTF TRUE
#define DEBUG2_USING_PRINTF FALSE

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
#define PATH_CHOOSER 0
#define OUT 1
#define ZERO 2
#define ONE 3
#define IN 4

// reserved constants for instruction index (e.g. cur_instruction, paths[0], paths[1], etc.)
#define PROGRAM_START_INDEX 0
#define EXIT_INDEX -1
instruction_index_type cur_instruction = PROGRAM_START_INDEX;

// *********************************************************

// program: bit copy
instruction_type prog_bitcopy[] =	{
	{ {OUT, IN}, {0,0} },
};

// program: bit copy with exit or repeat
instruction_type prog_bitcopy_exit[] =	{
	{ {OUT, IN}, {1,1} },
	{ {PATH_CHOOSER, IN}, {0, EXIT_INDEX} }
};

// program: NOT gate emulation
instruction_type prog_not[] =	{
	{ {PATH_CHOOSER, IN}, {2, 1} },
	{ {OUT, ZERO}, {0, 0} },
	{ {OUT, ONE}, {0, 0} }
};

// program: AND gate emulation
instruction_type prog_and[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 2} },
	{ {OUT, ZERO}, {0, 0} },
	{ {OUT, ONE}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {2, 3} }
};

// program: OR gate emulation
instruction_type prog_or[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 3} },
	{ {OUT, ZERO}, {0, 0} },
	{ {OUT, ONE}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {3, 3} }
};

// program: NAND gate emulation
instruction_type prog_nand[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 2} },
	{ {OUT, ONE}, {0, 0} },
	{ {OUT, ZERO}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {2, 3} }
};

// program: NOR gate emulation
instruction_type prog_nor[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 3} },
	{ {OUT, ONE}, {0, 0} },
	{ {OUT, ZERO}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {3, 3} }
};

// program: memory to output
instruction_type prog_memory_out[] =	{
	{ {10,	11}, {1, 1} },
	{ {13,	14}, {2, 2} },
	{ {OUT,	10}, {3, 3} },
	{ {OUT,	11}, {4, 4} },
	{ {OUT,	12}, {5, 5} },
	{ {OUT,	13}, {6, 6} },
	{ {OUT,	14}, {EXIT_INDEX, EXIT_INDEX} }
};

// program: output compare test
instruction_type prog_compare_test[] =	{
	{ {OUT,	ZERO}, {1, 1} },
	{ {OUT,	ONE}, {2, 2} },
	{ {OUT,	ZERO}, {EXIT_INDEX, EXIT_INDEX} }
};

// *********************************************************

// use "program selector" to select which program to run in the Machine
instruction_type* prog_selector = prog_nor;

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
#define QUIT_SIGNAL -2

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
		return QUIT_SIGNAL;
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
		if(TRUE == SKIP_INPUT_REQUEST){
			//printf(" @skipped");
			return 0;
		}else
		while(TRUE){
			value = getbit();
			if(value == INVALID_BIT){
				printf(" [insert bit (type '0' or '1') or quit (type 'q')!] ");
			} else if(value == QUIT_SIGNAL){
				return QUIT_SIGNAL;
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

int perform_operation(){
	instruction_type instruction = prog_selector[(long long)cur_instruction];
	int bit = read_bit_from_address(instruction.mapping[1]);
	if(bit==QUIT_SIGNAL) return QUIT_SIGNAL;
	else
		return write_bit_to_address(instruction.mapping[0], bit);
}

void path_choice(){
	instruction_type instruction = prog_selector[(long long)cur_instruction];
	int selector_bit = read_bit_from_memory(PATH_CHOOSER);
	cur_instruction = instruction.paths[selector_bit];
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

#define STEPS_LIMIT 10

int run_program(instruction_type* run_this){
	const char* target_output = "010"; // can be set to NULL to disable

	prog_selector = run_this;
	
	int pause = FALSE;
	
	printf(" { ");
	
	// counters for stats
	int cycle_counter = 0;
	int output_counter = 0;
	
	cur_instruction = PROGRAM_START_INDEX;
	const char * target_output_cur = target_output;
	while(TRUE){
		if(cur_instruction==EXIT_INDEX){
			printf(" @EXIT_INDEX");
			break;
		}
	
		const int out = perform_operation();
	
		if(out == QUIT_SIGNAL){
			printf(" @QUIT_SIGNAL");
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
						pause = TRUE;
						break;
					}
				
				}else{
					break;
				}
			}else{
				output_counter++;			
			}
			
			if(FALSE == DEBUG_USING_PRINTF)
				printf("%d", out);
		}
	
		path_choice();
		
		cycle_counter++; if(cycle_counter>=STEPS_LIMIT) break;
	}
	
	printf(" } \n");
	
	if(pause == TRUE) PAUSE();
	
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

					return TRUE; // overflow, full iteration cycle
				}
			}

		}	
	}
	return FALSE;
}

int next_program(instruction_type program[], const int size){

	int instruction_index = 0; // instruction index
	int overflow;
	
	while(TRUE){
		overflow = increment_instruction(&program[instruction_index]);
		
		if(overflow){
			instruction_index++;

			if(size <= instruction_index){
				overflow = TRUE; // full iteration completed
				break;
			}
		}else{
			overflow = FALSE; // full iteration not completed
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
	const int program_size = 4;
	current_program = (instruction_type*) malloc(sizeof(instruction_type)*program_size);

	// stats
	int count_programs = 0;
	int max_oc = -1;
	
	while(TRUE){ // iterate programs
		
		// - run program
		
		print_program(current_program, program_size);
		
		reset_memory();
		int output_counter = run_program(current_program);
		
		// output stats
		printf(" (OC:%d)\n", output_counter);
				
		if(output_counter>max_oc){
			max_oc = output_counter;
		}
		
		count_programs++; // count programs

		if(TRUE == next_program(current_program, program_size)){				
			break; // iteration completed
		}
	}
	
	printf("(count_programs:%d)", count_programs);
	
	return max_oc;
}

// ------------------------------------------------------
//  ---------------- SIMPLE 2 --------------------------

const char* target_output = "0101101110001000"; // can be set to NULL to disable
const int imodulo = 8; // 256, 16, 2

void reset_memory2(char memory[], int memory_size){
	// set all memory to zero
	memset(memory, 0, memory_size);
}

int run_program2(char memory_in[], int size){ // program_size
	
	char* memory = (char*) malloc(sizeof(char)*size);
	memcpy(memory, memory_in, sizeof(char)*size);
	
	const char * target_output_cur = target_output;
	
	int output_counter = 0;
	
	int i=0;
	
	int steps = 0;
	
	while(1){
		char delta = memory[i];
		if(delta == 0){
			
			const int out = memory[(i+1)%size] &1;
			
			printf(" %d ", out);
			
			// there is output
			
			if(target_output!=NULL){
				const char cur = *target_output_cur;
				
				const int cur_bit = cur-'0';
				assert(cur_bit==0 || cur_bit==1);
				
				if(out==cur_bit){
				
					output_counter++;
					target_output_cur++;
					if(*target_output_cur=='\0'){
						break;
					}
				
				}else{
					break;
				}
			}else{
				output_counter++;			
			}
			
			delta = 1;
		}else{
			memory[i] = (memory[i]+memory[(i+1)%size]) %imodulo;
		}
		
		delta -= imodulo/2; // forward and backward
		
		i = (i+delta) %size;
		
		const int max_steps = 10;
		steps++; if(steps>=max_steps){ printf(" @max_steps"); break; }
	}
	
	return output_counter;
	
}

int increment_instruction2(char* instruction){
	*instruction = (*instruction + 1) %imodulo;
	return *instruction==0;
}

int next_program2(char program[], const int program_size){

	int instruction_index = 0; // instruction index
	int overflow;
	
	while(TRUE){
		overflow = increment_instruction2(&program[instruction_index]);
		
		if(TRUE == overflow){
			instruction_index++;

			if(program_size <= instruction_index){
				overflow = TRUE; // full iteration completed
				break;
			}
		}else{
			overflow = FALSE; // full iteration not completed
			break;	
		}
	}
	
	return overflow;
}

void print_program2(char current_program[], int program_size, FILE* file){
	for(int i=0; i<program_size; i++)
		fprintf(file, " %d ", current_program[i]);
	fprintf(file, "\n");
}


int iterate_programs2(){
	
	FILE* fp = fopen("iprog.txt", "w+");
	
	// program
	char* current_program;
	const int PROGRAM_SIZE = 50;
	current_program = (char*) malloc(sizeof(char)*PROGRAM_SIZE);

	// stats
	int count_programs = 0;
	int max_oc = -1;
	
	while(TRUE){ // iterate programs
		
		// - run program
		
		print_program2(current_program, PROGRAM_SIZE, stdout);
		
		int output_counter = -1;
		output_counter = run_program2(current_program, PROGRAM_SIZE);
		
		// output stats
		printf(" (OC:%d)\n", output_counter);
		
		// pause
		const int string_size = strlen(target_output);
		if(string_size-output_counter<=3) PAUSE();
				
		if(output_counter>max_oc){
			
			// new max_oc value
			max_oc = output_counter;
			
			// - log
			
			// log program
			print_program2(current_program, PROGRAM_SIZE, fp);
			
			// log output
			for(int i=0; i<max_oc; i++){
				fprintf(fp, "%c", target_output[i]);
			}
			fprintf(fp, "\n");
			
			fflush(fp);
			
			// - reset and PAUSE
			char reset = 'd';
			reset = PAUSE();
			if(reset=='r') max_oc=0;
		}
		
		count_programs++; // count programs

		if(TRUE == next_program2(current_program, PROGRAM_SIZE)){				
			break; // iteration completed
		}
	}
	
	printf("(count_programs:%d)", count_programs);
	
	return max_oc;
}

int main(int argc, char **argv) {
	//multiple_programs_executed_sequentially();
	
	//printf("enter to run\n"); PAUSE();
	
	const int max_oc = iterate_programs2();
	
	//printf("(out stats:%d)", run_program(prog_compare_test));
	
	printf(" (maxOutC:%d)", max_oc);
	
	return 0;
}
