/*    Simulated workload using OpenMP

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
#include <limits.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>

#define LOCKED 0
#define UNLOCKED 1

// 500 million iterations should take several seconds to run
#define ITERS 500000000

volatile int lk = UNLOCKED;

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

void lock(volatile int *ptr) {
	int ret_val = __atomic_sub_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	while (ret_val < 0) {
		//sleep
		__atomic_store_n(ptr, -1, __ATOMIC_RELEASE);
	    syscall(SYS_futex, ptr, FUTEX_WAIT, -1, NULL);	
		ret_val = __atomic_sub_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	}
	usleep(1);
	printf("CPU %d holds the lock\n", sched_getcpu());
}

void unlock(volatile int *ptr) {
	int ret_val = __atomic_add_fetch(ptr, 1, __ATOMIC_ACQ_REL);
	if (ret_val != 1) {
		__atomic_store_n(ptr, 1, __ATOMIC_RELEASE);
		syscall(SYS_futex, ptr, FUTEX_WAKE, INT_MAX);
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
