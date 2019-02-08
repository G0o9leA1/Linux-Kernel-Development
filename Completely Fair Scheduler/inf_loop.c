/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#define _GNU_SOURCE 
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("ERROR! Program should take a single integer argument\n");
		exit(-1);
	}
	int cpu_num = atoi(argv[1]); // CPU core number
	cpu_set_t cpu;
	CPU_ZERO(&cpu); // initialize cpu_set_t variable
	CPU_SET(cpu_num, &cpu); // set cpu_set_t variable
	sched_setaffinity(0, sizeof(cpu), &cpu); // specify the set of allowable CPUs
	while (1) {}
	return 0;
}
