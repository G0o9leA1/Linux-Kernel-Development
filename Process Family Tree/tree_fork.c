/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
	if (argc != 2) // check command line parameter
       	{
		printf("Error! You should provide an integer parameter between 1-10\n");
		exit(0);
	}
	int generations = atoi(argv[1]);
	if (generations < 1 || generations > 10) // check validation of generations
	{
		printf("Error! Integer parameter should be between 1-10\n");
		exit(0);
	}	
	int gen = 1; 
	pid_t rv;
	while (1)
	{
		if (gen == 1 || rv == 0) // print every process only once
			printf("This is the generation: %d, my PID: %u\n", gen, getpid());
		gen++;
		if (gen > generations+1)
		       return 0;
		else 
		{
			rv = fork(); // spawn a child process
			wait(0); // wait for complete
			if (rv < 0) // spawn failed
				return -1;
		}
	}
	return 0;
}
