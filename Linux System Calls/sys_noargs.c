/* This file implements a demonstration syscall for an OS course. It
 * *  takes no arguments and prints out a simple message to the kernel
 * *  log, indicating that it was called.
 * *
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

//Define a system call implementation that takes no arguments
SYSCALL_DEFINE0( noargs ){
//
//  // print out a simple message indicating the function was called, and return SUCCESS
    printk("Someone invoked the sys_noargs system call");
    return 0;  
}
//      //End of file
