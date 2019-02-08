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

// default values for module parameters
static unsigned long log_sec = 1;
static unsigned long log_nsec = 0;
// module parameters
module_param(log_sec, ulong, 0);
module_param(log_nsec, ulong, 0);

static ktime_t kt; // timer interval
static struct hrtimer timer; // timer
static struct task_struct *task; // task struct for kernel thread

static enum hrtimer_restart hrtimer_func(struct hrtimer *timer)
{
	wake_up_process(task); // wake up the kernel thread
	hrtimer_forward_now(timer, kt); // reschedule the timer's next expiration
	printk(KERN_INFO "Timer restart again!\n");
	return HRTIMER_RESTART;
}

static int kthread_func(void *f)
{
	printk(KERN_ALERT "kthread_func is running\n");
	while (1) 
	{
		printk(KERN_DEBUG "Another iteration in kthread function has started, nvcsw: %lu, nivcse: %lu\n", current->nvcsw, current->nivcsw);
		set_current_state(TASK_INTERRUPTIBLE); // set state to TASK_INTERRUPTIBLE
		schedule(); // suspend its execution
		if (kthread_should_stop())
			break;
	}
	printk(KERN_ALERT "kthread_func is terminating\n");
	return 0;
}

static int timer_init(void)
{
	printk(KERN_ALERT "module initialized\n");
	kt = ktime_set(log_sec, log_nsec); // set timer interval
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL); // initialize timer
	timer.function = hrtimer_func; // set timer function
	hrtimer_start(&timer, kt, HRTIMER_MODE_REL); // start timer
    task = kthread_run(kthread_func, NULL, "single_kthread"); // spawn a kernel thread
    return 0;
}

static void timer_exit(void)
{
	// cancel timer
	hrtimer_cancel(&timer);
	// alert the thread that it should stop
	if (!IS_ERR(task))
    	kthread_stop(task);
	printk(KERN_ALERT "module is being unloaded\n");
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("GLW");
MODULE_DESCRIPTION ("422 Lab1");
