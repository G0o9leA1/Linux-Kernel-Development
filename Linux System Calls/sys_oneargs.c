/* This file implements a demonstration syscall for an OS course. It
 * *  takes one argument and prints out a simple message to the kernel
 * *  log, indicating that it was called.
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

//Define a system call implementation that takes no arguments
SYSCALL_DEFINE1(oneargs, int, oneargss ){
//
//  // print out a simple message indicating the function was called, and return SUCCESS
    printk("Someone invoked the sys_oneargs system call, the para is %d\n",oneargss);
    return 0;  
}
//End of file
