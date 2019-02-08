/*    Simulated workload using OpenMP
 *
 * Author: Chengyue Gong, Zixuan Li, Wentao Wang
 * Orgnization: Washington University in St. Louis
 * 
 * This program will create some number of seconds of work on each processor
 * on the system.
 *
 * This program requires the use of the OpenMP compiler flag, and that 
 * optimizations are turned off, to build correctly. E.g.: 
 * gcc -fopenmp workload.c -o workload
 */

#define _GNU_SOURCE
#include <stdio.h> // for printf()
#include <sched.h> // for sched_getcpu()
#include <stdbool.h> // for true and false
#include <unistd.h> // for usleep()

#define LOCKED 0
#define UNLOCKED 1

// 500 million iterations should take several seconds to run
#define ITERS 500000000

volatile int lk = UNLOCKED;

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

bool compare_exchange(volatile int* ptr, int expected, int desired) {
	int *pe = &expected;
	int *pd = &desired;
       	if (__atomic_compare_exchange(ptr, pe, pd, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) return true;
	else return false;
}	

void lock(volatile int *plock) {
	while (!compare_exchange(plock, UNLOCKED, LOCKED));
	usleep(1);
	printf("CPU %d holds the lock\n", sched_getcpu());
}

void unlock(volatile int *plock) {
	if (!compare_exchange(plock, LOCKED, UNLOCKED)) {
		printf("Error: CPU %d attempted to release the lock without holding it\n", sched_getcpu());
		return;
	}
	printf("CPU %d releases the lock\n", sched_getcpu());
}

int main (int argc, char* argv[]){
	// Create a team of threads on each processor
	#pragma omp parallel
	{
		// Each thread executes this code block independently
		lock(&lk);
		critical_section();
		unlock(&lk);
		printf("CPU %d finished!\n", sched_getcpu());
	}

	return 0;
}
