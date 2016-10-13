#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#endif

char memory[] = {0,0,0,0,0,0,0,0,0,0};

typedef struct {
	long mapping[2];
	long paths[2];
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

// demo0: bit copy
instruction_type instructions_demo0[] =	{
	{ {OUT, IN}, {0,0} },
};

// demo1: bit copy with exit or repeat
instruction_type instructions_demo1[] =	{
	{ {OUT, IN}, {1,1} },
	{ {PATH_CHOOSER, IN}, {0, EXIT} }
};

// demo2: NOT gate emulation
instruction_type instructions_demo2[] =	{
	{ {PATH_CHOOSER, IN}, {2, 1} },
	{ {OUT, ZERO}, {0, 0} },
	{ {OUT, ONE}, {0, 0} }
};

// demo3: AND gate emulation
instruction_type instructions_demo3[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 2} },
	{ {OUT, ZERO}, {0, 0} },
	{ {OUT, ONE}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {2, 3} }
};

// demo4: OR gate emulation
instruction_type instructions_demo4[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 3} },
	{ {OUT, ZERO}, {0, 0} },
	{ {OUT, ONE}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {3, 3} }
};

// demo5: NAND gate emulation
instruction_type instructions_demo5[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 2} },
	{ {OUT, ONE}, {0, 0} },
	{ {OUT, ZERO}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {2, 3} }
};

// demo6: NOR gate emulation
instruction_type instructions_demo6[] =	{
	{ {PATH_CHOOSER, IN}, {1,4} },
	{ {PATH_CHOOSER, IN}, {2, 3} },
	{ {OUT, ONE}, {0, 0} },
	{ {OUT, ZERO}, {0, 0} },
	{ {PATH_CHOOSER, IN}, {3, 3} }
};

// "program selector"
// - use "program selector" to select which program to run in the RTM
instruction_type* instructions = instructions_demo6;

#ifndef _WIN32
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
	char ch;
	#ifdef _WIN32
		ch = _getche();
	#else
		ch = getch();
	#endif
	if (ch=='0' || ch=='1') return ch-'0';
	else if (ch=='q'){
		printf(" ) ");
		exit(0);
	}
	else return -1;
}

int getinput(long* mapping, int index){
	int value;
	if(mapping[index]==IN) {
		 while( (value = getbit()) == -1 ){
			printf(" !insert bit (type '0' or '1') or quit! (type 'q') ");
		}
	} else if(mapping[index]==ZERO) {
		value = 0;
	} else if(mapping[index]==ONE) {
		value = 1;
	}
	memory[mapping[index]] = value;
	return value;
}

void perform_operation(){
	instruction_type instruction = instructions[current_op];
	memory[instruction.mapping[0]] = getinput(instruction.mapping, 1);	
	if(instruction.mapping[0]==OUT)printf("%d",memory[OUT]);
}

void path_choice(){
	instruction_type instruction = instructions[current_op];
	current_op = instruction.paths[
		(int) memory[PATH_CHOOSER]
	];
}

int main(int argc, char **argv) {
	printf(" ( ");
	while(1){
		if(current_op==EXIT) break;
		perform_operation();
		path_choice();
	}
	printf(" ) ");
	return 0;
}
