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

int main()
{
	printf ("syscall to getuid returned: %u\n", syscall(__NR_getuid));

	int return_val = syscall(__NR_setuid,0);
	printf ("syscall to setuid returned: %d\n", syscall(__NR_setuid,0));

	if( return_val != 0 ) 
		printf("Error: setuid failed! Reason: %s\n", strerror(errno));

	return 0;
}
