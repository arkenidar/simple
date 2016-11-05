#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define DEBUG_USING_PRINTF 0

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

typedef struct instruction_type_struct{
	char mapping[2]; // 8 bit data addressing (256 bits of data memory)
	char paths[2]; // 8 bit instruction addressing (256 instructions)
} instruction_type;

// reserved constants for mapping[0] or mapping[1]
#define PATH_CHOOSER 0
#define OUT 1
#define IN 2
#define ZERO 3
#define ONE 4

// reserved constants for current_op, paths[0] or paths[1]
#define EXIT -1
long current_op = 0;

// program: bit copy
instruction_type prog_bitcopy[] =	{
	{ {OUT, IN}, {0,0} },
};

// program: bit copy with exit or repeat
instruction_type prog_bitcopy_exit[] =	{
	{ {OUT, IN}, {1,1} },
	{ {PATH_CHOOSER, IN}, {0, EXIT} }
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
	{ {OUT,	14}, {EXIT, EXIT} }
};

// use "program selector" to select which program to run in the Machine
instruction_type* prog_selector = prog_nor;

void debug_newline(){
	if(DEBUG_USING_PRINTF)
		printf("\n");
}

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

int getbit(){
	
	printf(" in:");
	
	char ch;
	#if INPUT_SYSTEM == INPUT_SYSTEM_WINDOWS
		ch = _getche();
	#endif
	#if INPUT_SYSTEM == INPUT_SYSTEM_UNIX
		ch = getch();
	#endif
	if (ch=='0' || ch=='1'){
		int bit = ch=='0'?0:1; // '0' or '1'
		if(DEBUG_USING_PRINTF)
			printf("[getbit(): %d]", bit);
		return bit;
	}
	else if (ch=='q'){
		printf(" } ");
		exit(0);
	}
	else return -1;
}

#define BitVal(data,y)   ( (data>>y) & 1)  /** Return Data.Y value **/
#define ClearBit(data,y) data &= ~(1 << y) /** Clear Data.Y to 0   **/
#define SetBit(data,y)   data |= (1 << y)  /** Set Data.Y to 1     **/

#define bitArray_wordSize (sizeof(bit_type)*8)
#define bitArray_wordIndex(bit_address) (bit_address/bitArray_wordSize)
#define bitArray_bitIndex(bit_address)  (bit_address%bitArray_wordSize)
//#define bitArray_wordIndex(bit_address) (bit_address>>bitArray_wordSize)
//#define bitArray_bitIndex(bit_address)  (bit_address&&((1<<bitArray_wordSize)-1))

void write_bit_to_memory(int write_to, int bit){

	#if BIT_ACCESS_IN_USE == SIMPLE_BIT_ACCESS
		memory[write_to] = bit;
	#endif

	#if BIT_ACCESS_IN_USE == BITWISE_OPS_BIT_ACCESS
		if(DEBUG_USING_PRINTF)
			printf("[write_btm(): %d]", bit);
	
		int x = bitArray_wordIndex(write_to);
		int y = bitArray_bitIndex(write_to);
		
		if(bit==0)
			ClearBit(memory[x],y);
		else if(bit==1)
			SetBit(memory[x],y);
	#endif
}

int read_bit_from_memory(int read_from){
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

int read_bit_from_address(int read_from){
	int value;
	if(read_from==IN) {
		 while( (value = getbit()) == -1 ){
			printf(" [insert bit (type '0' or '1') or quit (type 'q')!] ");
		}
	} else if(read_from==ZERO) {
		value = 0;
	} else if(read_from==ONE) {
		value = 1;
	} else {
		value = read_bit_from_memory(read_from);
	}
	
	if(DEBUG_USING_PRINTF)
		printf("[read_bfa(): %d]", value);
	
	return value;
}

void write_bit_to_address(int write_to, int bit){
	write_bit_to_memory(write_to, bit);
	
	if(write_to==OUT){
		if(DEBUG_USING_PRINTF)
			printf("[out_wbta(): %d]", bit);
		printf(" out:%d", bit);
	}
}

void perform_operation(){
	instruction_type instruction = prog_selector[current_op];
	int bit = read_bit_from_address(instruction.mapping[1]);
	write_bit_to_address(instruction.mapping[0], bit);
}

void path_choice(){
	instruction_type instruction = prog_selector[current_op];
	int bit = read_bit_from_memory(PATH_CHOOSER);
	current_op = instruction.paths[bit];
}

int test_bit_array(){
	int bits[] = {0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0};
	int size = sizeof(bits)/sizeof(int);
	int i;
	for(i = 0; i<size; i++)
		write_bit_to_memory(10+i, bits[i]);
	
	debug_newline();
	
	for(i = 0; i<size; i++)
		if(read_bit_from_memory(10+i)!=bits[i])
			return 0;
	return 1;	
}

int main(int argc, char **argv) {
	
	if(DEBUG_USING_PRINTF)
		printf("- bitArray_wordSize: %d\n", (int)bitArray_wordSize);
	
	int bit_array_works = test_bit_array();
	assert(bit_array_works); 
	
	if(DEBUG_USING_PRINTF)
		printf("- test_bit_array(): %d\n", bit_array_works);
	
	printf(" { ");
	while(1){
		if(current_op==EXIT) break;
		perform_operation();
		path_choice();
	}
	printf(" } ");

	return 0;
}
