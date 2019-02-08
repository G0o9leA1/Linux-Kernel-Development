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

int main()
{
	printf ("getuid returned: %d\n", getuid());

	int return_val = setuid(0);
	printf ("setuid returned: %d\n", setuid(0));

	if( return_val != 0 ) printf("Error: setuid failed! Reason: %s\n", strerror(errno));

	return 0;
}
