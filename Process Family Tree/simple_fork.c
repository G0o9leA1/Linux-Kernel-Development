/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	printf("Before the fork, My PID: %u\n", getpid());
	pid_t rv = fork(); // spawn a child process
	if (rv == 0) // child process
	{
		printf("---------------------\n");
		printf("This is child process!\n");
		printf("My PID: %u\n", getpid());
		printf("My parent's PID: %u\n", getppid());
		printf("---------------------\n");
	}	
	else // parent process
	{
		printf("My child's PID: %u\n", rv);
	}
	return 0;
}
