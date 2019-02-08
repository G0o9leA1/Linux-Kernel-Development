/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <stdio.h>
#include <time.h>
#define CLOCK_SIZE 6

int main() 
{
	struct timespec tp;
	clockid_t clocks[CLOCK_SIZE] = {CLOCK_REALTIME,
	       				CLOCK_REALTIME_COARSE,	
			     		CLOCK_MONOTONIC,
				       	CLOCK_MONOTONIC_COARSE,	
			     		CLOCK_PROCESS_CPUTIME_ID,
			     		CLOCK_THREAD_CPUTIME_ID};
	int i;
	int res_ret;
	for (i = 0; i < CLOCK_SIZE; i++)
	{
		res_ret = clock_getres(clocks[i], &tp);
		if (res_ret == 0) // succeed
		{
			printf("clock = %d, sec = %ld, nsec = %ld\n", clocks[i], tp.tv_sec, tp.tv_nsec);
		}
		else // fail
		{
			printf("ERROR\n");
		}
	}
	return 0;
}
