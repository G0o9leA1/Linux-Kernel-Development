/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/timekeeping.h>
#include <linux/slab.h>
#include <asm/spinlock.h>
#define SYNCTIME 2

// default values for module parameters
static unsigned long num_threads = 1; // number of threads
static unsigned long upper_bound = 10; // upper bound
// module parameters
module_param(num_threads, ulong, 0);
module_param(upper_bound, ulong, 0);

struct task_struct **task; // kernel thread
struct timespec ts0, ts1, ts2; // time stamp

int *counter_array; // a pointer to an array of counter variables
atomic_t *num_array; // a pointer to an array of integers
volatile int curnt; // position of the current number
atomic_t finished; // whether or not the computation of prime nubers has finished in each thread
volatile int threads_remained; // a counter for how many threads need to synchronize
DEFINE_SPINLOCK(lock); // a lock

static struct timespec get_time_diff(struct timespec *t1, struct timespec *t0) {
	struct timespec diff;
	diff.tv_sec = (*t1).tv_sec - (*t0).tv_sec;
	diff.tv_nsec = (*t1).tv_nsec - (*t0).tv_nsec;
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += 1000000000L;
		diff.tv_sec--;
	}
	return diff;
}

static void barrier_sync(int time) {
	if (time > SYNCTIME) {
		printk(KERN_ALERT "Error in barrier_sync(): invalid time\n");
		return;
	}
	//printk(KERN_DEBUG "Reach barrier %d\n", time); // debug msg
	spin_lock(&lock);
	threads_remained--;	
	if (threads_remained == num_threads*(SYNCTIME-time)) {
		if (time == 1) ktime_get_ts(&ts1);
		else if (time == 2) ktime_get_ts(&ts2);
	}
	spin_unlock(&lock);
	while (threads_remained > num_threads*(SYNCTIME-time));
}

static void cross_out(void *counter) {
	int cur, step;
	while (1) {
		spin_lock(&lock);
		// store the value of the current position
		cur = curnt;
		// select the next number
		curnt++;
		while (curnt < upper_bound-1 && atomic_read(&num_array[curnt]) == 0) {
			curnt++;
		}	
		spin_unlock(&lock);
		// check the local variable
		if (cur >= upper_bound-1) return;
		// Sieve of Eratosthenes
		step = cur+2;
		cur += step;
		//printk(KERN_DEBUG "Crossing out multiples of %d\n", step); // debug msg
		while (cur < upper_bound-1) {
			atomic_set(&num_array[cur], 0);
			*(int *)counter += 1;
			//printk(KERN_DEBUG "%d has been crossed out\n", cur+2); // debug msg
			cur += step;
		}
	}
}

static int kthread_func(void *counter) {
	barrier_sync(1);
	cross_out(counter);
	barrier_sync(2);
	atomic_inc(&finished);
	//printk(KERN_DEBUG "Thread pid: %u Done!\n", current->pid); // debug msg
	return 0;
}

static int my_init(void)
{
	int i;
	printk(KERN_ALERT "Module initialized\n");
	// get time stamp
	ktime_get_ts(&ts0);
	// initialization of all variables
	counter_array = 0;
	num_array = 0;
	task = 0;
	atomic_set(&finished, num_threads); // completed
	
	if (num_threads < 1 || upper_bound < 2) {
		printk(KERN_ALERT "Error in init(): Invalid module parameter!\n");
		if (num_threads < 1)
			printk(KERN_ALERT "Thread number %lu < 1\n", num_threads);
		if (upper_bound < 2)
			printk(KERN_ALERT "Upper bound %lu < 2\n", upper_bound);
		return 0;
	}
	// allocate kernel memory for the array of integers
	num_array = (atomic_t *) kmalloc(sizeof(atomic_t)*(upper_bound-1), GFP_KERNEL);
	if (!num_array) {
		printk(KERN_ALERT "Error in init(): The first kmalloc failed!\n");
		return 0;
	}
	// allocate kernel memory for the counter array
	counter_array = (int *) kmalloc(sizeof(int)*num_threads, GFP_KERNEL);
	if (!counter_array) {
		printk(KERN_ALERT "Error in init(): The second kmalloc failed!\n");
		return 0;
	}
	// allocate kernel memory for tasks
	task = (struct task_struct **) kmalloc(sizeof(struct task_struct)*num_threads, GFP_KERNEL);
	if (!task) {
		printk(KERN_ALERT "Error in init(): The third kmalloc failed!\n");
		return 0;
	}
	// set counter variables
	for (i = 0; i < num_threads; i++) {
		counter_array[i] = 0;
	}
	// set array of integers from 2 to upper bound
	for (i = 0; i < upper_bound-1; i++) {
		atomic_set(&num_array[i], i+2);
	}
	curnt = 0; // num_array[0] = 2
	threads_remained = num_threads*SYNCTIME;
	atomic_set(&finished, 0); // not completed
	// spawn kernel threads
	for (i = 0; i < num_threads; i++) {
    		task[i] = kthread_run(kthread_func, &counter_array[i], "kthread%d", i);
	}
	return 0;
}

static void clear_memory(void) {
	kfree(task);
	kfree(num_array);
	kfree(counter_array);
	task = 0;
	num_array = 0;
	counter_array = 0;
}

static void print_statistics(void) {
	int i;
	int prime_count = 0; //num of prime number
	int nonprime_count = 0; //num of non-prime number
	int total_crossout = 0; //num of total crossout
	int waste_crossout = 0; //num of unnecessarily cross out
	struct timespec diff1, diff2, total; //time difference of ts1 - ts0 and ts2 - ts1

	for (i = 0; i < upper_bound-1; i++) {
		if (atomic_read(&num_array[i]) != 0) {
			prime_count++;
			printk(KERN_CONT "%d ", atomic_read(&num_array[i]));
			if (prime_count % 8 == 0) {
				printk(KERN_CONT "\n");
			}
		}
	}
	nonprime_count = upper_bound-1-prime_count;
	for (i = 0; i < num_threads; i++) {
		total_crossout += counter_array[i];
		//printk(KERN_DEBUG "Thread %d crossed out %d times\n", i, counter_array[i]); //debug msg
	}
	waste_crossout = total_crossout - nonprime_count;
	printk(KERN_INFO "Prime numbers: %d\n" , prime_count);
	printk(KERN_INFO "Non-prime numbers: %d\n" , nonprime_count);
	printk(KERN_INFO "Unnecessary crossing out: %d\n" , waste_crossout);
	printk(KERN_INFO "Upper bound = %lu, Number of threads = %lu\n", upper_bound, num_threads);
	diff1 = get_time_diff(&ts1, &ts0);
	diff2 = get_time_diff(&ts2, &ts1);
	total = get_time_diff(&ts2, &ts0);
	printk(KERN_INFO "Time for setting up the module: %ld.%.9ld\n", diff1.tv_sec, diff1.tv_nsec);
	printk(KERN_INFO "Time for processing primes: %ld.%.9ld\n", diff2.tv_sec, diff2.tv_nsec);
	printk(KERN_INFO "Total time: %ld.%.9ld\n", total.tv_sec, total.tv_nsec);
}

static void my_exit(void)
{	
	printk(KERN_ALERT "Module unloaded\n");
	if (atomic_read(&finished) != num_threads) {
		printk(KERN_ALERT "Error in exit(): computation not finished\n");
		return;
	}
	if (!num_array) {
		printk(KERN_INFO "Allocation failed for num_array\n");
		return;
	}
	if (!counter_array) {
		printk(KERN_INFO "Allocation failed for counter_array\n");
		kfree(num_array);
		num_array = 0;
		return;	
	}
	if (!task) {
		printk(KERN_INFO "Allocation failed for task\n");
		kfree(num_array);
		kfree(counter_array);
		num_array = 0;
		counter_array = 0;
		return;
	}
	print_statistics();	
	clear_memory();
	return;
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("GLW");
MODULE_DESCRIPTION ("422 Lab2");
