/* simple_module.c - a simple template for a loadable kernel module in Linux,
* based on the hello world kernel module example on pages 338-339 of Robert
* Love's "Linux Kernel Development, Third Edition."

* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/* init function - logs that initialization happened, returns success */
static int 
simple_init(void)
{
    printk(KERN_ALERT "simple module initialized\n");
    printk("Jiffies value: %lu\n", jiffies);
    return 0;
}

/* exit function - logs that the module is being removed */
static void 
simple_exit(void)
{
    printk(KERN_ALERT "simple module is being unloaded\n");
    printk("Jiffies value: %lu\n", jiffies);
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("LKD Chapter 17");
MODULE_DESCRIPTION ("Simple CSE 422 Module Template");
