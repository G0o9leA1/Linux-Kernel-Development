/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/kthread.h>
#define N 4

// default values for module parameters
static unsigned long log_sec = 1;
static unsigned long log_nsec = 0;
// module parameters
module_param(log_sec, ulong, 0);
module_param(log_nsec, ulong, 0);

static ktime_t kt; // timer interval
static struct hrtimer timer; // timer
static struct task_struct *task[N]; // task struct for kernel thread

static enum hrtimer_restart hrtimer_func(struct hrtimer *timer)
{
	int i;
	// wake up the kernel threads
	for (i = 0; i < N; i++)
		wake_up_process(task[i]);
	hrtimer_forward_now(timer, kt); // reschedule the timer's next expiration
	printk(KERN_INFO "Timer restart again!\n");
	return HRTIMER_RESTART;
}

static int kthread_func(void *f)
{
	printk(KERN_ALERT "kthread_func is running on CPU%d\n", smp_processor_id());
	while (1) 
	{
		printk(KERN_DEBUG "Another iteration in kthread function has started on CPU%d, nvcsw: %lu, nivcse: %lu\n", smp_processor_id(), current->nvcsw, current->nivcsw);
		set_current_state(TASK_INTERRUPTIBLE);
		schedule(); // suspend its execution
		if (kthread_should_stop())
			break;
	}
	printk(KERN_ALERT "kthread_func is terminating on CPU%d\n", smp_processor_id());
	return 0;
}

static int timer_init(void)
{
	int i;
	printk(KERN_ALERT "module initialized\n");
	kt = ktime_set(log_sec, log_nsec); // set timer interval
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL); // initialize timer
	timer.function = hrtimer_func; // set timer function
	hrtimer_start(&timer, kt, HRTIMER_MODE_REL); // start timer
	// spawn 4 kernel threads
	for (i = 0; i < N; i++)
	{
		task[i] = kthread_create(kthread_func, NULL, "kthread%d", i); // create a kernel thread
		kthread_bind(task[i], i); // bind a thread to a core
		wake_up_process(task[i]); // wake up it
	}
    return 0;
}

static void timer_exit(void)
{
	int i;
	// cancel timer
	hrtimer_cancel(&timer);
	// alert the threads that they should stop
	for (i = 0; i < N; i++)
	{
		if(!IS_ERR(task[i]))
			kthread_stop(task[i]);	
	}
	printk(KERN_ALERT "module is being unloaded\n");
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("GLW");
MODULE_DESCRIPTION ("422 Lab1");
