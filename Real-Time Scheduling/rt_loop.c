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
	const int loopTimes = 500000000;
	if (argc != 3)
	{
		printf("ERROR! Program should take two integer arguments\n");
		exit(-1);
	}
	int cpu_num = atoi(argv[1]); // CPU core number
	int realpr = atoi(argv[2]); // real-time priority
	if (realpr < sched_get_priority_min(SCHED_RR) || realpr > sched_get_priority_max(SCHED_RR))
		printf("Not in the PR range!\n");
	struct sched_param param;
	param.sched_priority = realpr;
	int ret = sched_setscheduler(0, SCHED_RR, &param); // set real-time priority
	if(ret == -1)
		perror("sched_setscheduler failed!\n");
	cpu_set_t cpu;
	CPU_ZERO(&cpu); // initialize cpu_set_t variable
	CPU_SET(cpu_num, &cpu); // set cpu_set_t variable
	sched_setaffinity(0, sizeof(cpu), &cpu); // specify the set of allowable CPUs
	int i = 0;
	while (i <= loopTimes) { i++; }
	return 0;
}
