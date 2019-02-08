/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/mm.h>

#include <asm/uaccess.h>

typedef struct{
    unsigned int array[8];
}datatype_t;

static uint nr_structs = 2000;
module_param(nr_structs, uint, 0644); 

static struct task_struct * kthread = NULL;
static struct page *pages = NULL;

static unsigned int nr_structs_per_page;
static unsigned int nr_pages;
static unsigned int order;

static unsigned int
my_get_order(unsigned int value)
{
    unsigned int shifts = 0;

    if (!value)
        return 0;

    if (!(value & (value - 1)))
        value--;

    while (value > 0) {
        value >>= 1;
        shifts++;
    }

    return shifts;
}

static int
thread_fn(void * data)
{
    int i,j,k;
    int size;
    datatype_t *dt = NULL, *this_struct = NULL;
    printk("Hello from thread %s. nr_structs=%u\n", current->comm, nr_structs);
    printk("page size is：%lu\n", PAGE_SIZE);
    size = sizeof(datatype_t);
    printk("size of datatype_t is: %d\n", size);
    nr_structs_per_page = PAGE_SIZE / size;
    printk("the number of datatype_t can fit in a single page is: %u\n", nr_structs_per_page);

    nr_pages = nr_structs / nr_structs_per_page;
    if (nr_structs % nr_structs_per_page > 0) 
	    nr_pages++;
    order = my_get_order(nr_pages);
    printk("nr_struct_per_page is: %u\n", nr_structs_per_page);
    printk("nr_pages is: %u\n", nr_pages);  
    printk("order is: %u\n", order);
    pages = alloc_pages(GFP_KERNEL, order);
    if (pages == NULL){
        printk("ERROR: alloc_pages() Failed");
        return -1;
    }

    //iterate through our phyiscal memory pages and set each integer value to a specific number
    for (i = 0; i < nr_pages; i++) {
        dt = (datatype_t *)__va(PFN_PHYS(page_to_pfn(pages + i)));
        for (j = 0; j < nr_structs_per_page; j++) {
            this_struct = dt + j;
            for (k = 0; k < 8; k++) {
                this_struct->array[k] = i*nr_structs_per_page*8 + j*8 + k;
                //print out the value of every element for which both j=0 and k=0
                // if (j == 0 && k == 0) {
                //     printk("The first element in the %dth page is %u", i, this_struct->array[k]);
                // }
            }
        }
    }

    while (!kthread_should_stop()) {
        schedule();
    }
    
    for (i = 0; i < nr_pages; i++) {
      dt = (datatype_t *)page_address(pages + i);
        for (j = 0; j < nr_structs_per_page; j++) {
            this_struct = dt + j;
            for (k = 0; k < 8; k++) {
                if (this_struct->array[k] != (i*nr_structs_per_page*8 + j*8 + k)) {
                    printk("Error： array[%d] in the %dth element in the %dth page is wrong!",k,j,i);
                }
            }
        }
    }
    printk("Success!\n");
    __free_pages(pages,order);
    return 0;
}

static int
kernel_memory_init(void)
{
    printk(KERN_INFO "Loaded kernel_memory module\n");

    kthread = kthread_create(thread_fn, NULL, "k_memory");
    if (IS_ERR(kthread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(kthread);
    }
    
    wake_up_process(kthread);


    return 0;
}

static void 
kernel_memory_exit(void)
{
    kthread_stop(kthread);
    printk(KERN_INFO "Unloaded kernel_memory module\n");
}

module_init(kernel_memory_init);
module_exit(kernel_memory_exit);

MODULE_LICENSE ("GPL");
