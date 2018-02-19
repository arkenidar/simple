#include<stdio.h>
int main(){
	int in = 1;              // var
	int out = 0;             // var

	if(in==0) goto out_1;    // if goto
	if(in==1) goto out_0;    // if goto

	out_0:                   // label
	out = 0;                 // assignment
	goto exit;               // goto

	out_1:                   // label
	out = 1;                 // assignment
	goto exit;               // goto

	exit:                    // label
	printf("out:%d\n", out); // stdout
}
