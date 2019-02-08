/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#define N 4
#define iters 1000000

struct task_struct *task[N];
volatile int shared_data = 0;

static int kthread_func(void *f)
{
	int i;
	printk(KERN_ALERT "kthread_func is running on CPU%d\n", smp_processor_id());
	while (1) {
		for (i = 0; i < iters; i++) {		
			shared_data++;
		}	
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if (kthread_should_stop()) break;
	}
	printk(KERN_ALERT "kthread_func is terminating on CPU%d\n", smp_processor_id());
	return 0;
}

static int my_init(void)
{
	int i;
	printk(KERN_ALERT "Module initialized\n");
	for (i = 0; i < N; i++) {
		task[i] = kthread_create(kthread_func, NULL, "kthread%d", i); // create a kernel thread
		kthread_bind(task[i], i); // bind a thread to a cpu
		wake_up_process(task[i]); // wake up it
	}
	return 0;
}

static void my_exit(void)
{
	int i;
	for (i = 0; i < N; i++) {
		kthread_stop(task[i]);
	}
	printk(KERN_INFO "The value of shared_data is %d\n", shared_data);
	printk(KERN_ALERT "Module unloaded\n");
}	

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("GLW");
MODULE_DESCRIPTION ("422 Studio10");
