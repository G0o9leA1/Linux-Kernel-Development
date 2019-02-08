/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <asm/unistd.h>
#define __NR_noargs 398
#define __NR_oneargs 399

int main()
{
	int return_val = syscall(__NR_noargs);
	if( return_val != 0 ) 
		printf("Error: failed! Reason: %s\n", strerror(errno));
	
	return_val = syscall(__NR_oneargs,666);
	if( return_val != 0 ) 
		printf("Error: failed! Reason: %s\n", strerror(errno));

	return 0;
}
