/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/memory.h>
#include <linux/mm.h>

#include <paging.h>

static atomic_t allocate_times;
static atomic_t free_times;
static unsigned int demand_paging = 1;
module_param(demand_paging, uint, 0644);

struct state {
	atomic_t counter;
	struct page **pages;
};

static unsigned long
get_nr_pages(struct vm_area_struct * vma) {
    unsigned long nr_pages = (vma->vm_end - vma->vm_start) / PAGE_SIZE;
    if ((vma->vm_end - vma->vm_start) % PAGE_SIZE > 0) nr_pages++;
    return nr_pages;
}

static unsigned int
my_get_order(unsigned int value)
{
	unsigned int shifts = 0;
	if (!value) return 0;
       	if (!(value & (value - 1))) value--;
	while (value > 0) {
		value >>= 1;
		shifts++;
	}	
	return shifts;
}

static int
do_fault(struct vm_area_struct * vma,
         unsigned long           fault_address)
{
        // allocate a new page of PM - alloc_page
        struct page *page = alloc_page(GFP_KERNEL);
    	printk(KERN_INFO "paging_vma_fault() invoked: took a page fault at VA 0x%lx\n", fault_address);
	    if (page == NULL) {
            // memory allocation fails
            return VM_FAULT_OOM;
	    }
        atomic_inc(&allocate_times);
    	// update the process' page tables to map the VA to new PA - remap_pfg_range
    	// remap_pfn_range(struct vm_area_struct * vma, unsigned long vaddr, unsigned long pfn, unsigned long length, vm_flags_t pg_prot);
    	// updates the page tables to map the VA starting at vaddr to the PA starting at page frame pfn
    	// for length bytes with access bits specified in pg_prot
    	if (remap_pfn_range(vma, PAGE_ALIGN(fault_address), page_to_pfn(page), PAGE_SIZE, vma->vm_page_prot)  == 0) {
    		// upadte is successful
            // remember the page
            struct state *ptr = (struct state *)vma->vm_private_data;
            ptr->pages[atomic_read(&ptr->counter)] = page; // store the PM
            atomic_inc(&ptr->counter); // increase counter
            // printk(KERN_DEBUG "counter = %d\n", atomic_read(&ptr->counter));
            return VM_FAULT_NOPAGE;
    	} else {
    		// fail to update
    		return VM_FAULT_SIGBUS;
   	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
static int
paging_vma_fault(struct vm_area_struct * vma,
                 struct vm_fault       * vmf)
{
    unsigned long fault_address = (unsigned long)vmf->virtual_address
#else
static int
paging_vma_fault(struct vm_fault * vmf)

{
    struct vm_area_struct * vma = vmf->vma;
    unsigned long fault_address = (unsigned long)vmf->address;
#endif

    return do_fault(vma, fault_address);
}

static void
paging_vma_open(struct vm_area_struct * vma)
{   
    struct state *ptr = (struct state *)vma->vm_private_data;
    printk(KERN_INFO "paging_vma_open() invoked\n");
    atomic_inc(&ptr->counter); 
    // printk(KERN_DEBUG "open: counter = %d\n", atomic_read(&ptr->counter));
}

static void
paging_vma_close(struct vm_area_struct * vma)
{
    int i;
    unsigned long nr_pages = get_nr_pages(vma);
    struct state *ptr = (struct state *)vma->vm_private_data;
    printk(KERN_INFO "paging_vma_close() invoked\n");
    // printk(KERN_DEBUG "close: counter = %d", atomic_read(&ptr->counter));
    if (atomic_read(&ptr->counter) == nr_pages) {            
        for (i = 0; i < nr_pages; i++) {    
            __free_page(ptr->pages[i]); // free PM
            atomic_inc(&free_times);
        }
        kfree(ptr->pages);
        kfree(ptr); // free tracker structure
    } else {
        printk(KERN_ALERT "close: failed to free physical memory and tracker structure\n");
    }
}

static struct vm_operations_struct
paging_vma_ops = 
{
    .fault = paging_vma_fault,
    .open  = paging_vma_open,
    .close = paging_vma_close
};

/* vma is the new virtual address segment for the process */
static int
paging_mmap(struct file           * filp,
            struct vm_area_struct * vma)
{
    int i;
    unsigned long nr_pages = get_nr_pages(vma);
    struct state *state_ptr = (struct state *)kmalloc(sizeof(struct state), GFP_KERNEL); // kmalloc struct state
    //printk(KERN_INFO "pages number = %lu\n", nr_pages);
    state_ptr->pages = (struct page **)kmalloc(sizeof(struct page *)*nr_pages, GFP_KERNEL);
    atomic_set(&state_ptr->counter, 0);
    vma->vm_private_data = (void *)state_ptr;

    /* prevent Linux from mucking with our VMA (expanding it, merging it 
    * with other VMAs, etc.)
    */
    vma->vm_flags |= VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_NORESERVE
          | VM_DONTDUMP | VM_PFNMAP;

    /* setup the vma->vm_ops, so we can catch page faults */
        vma->vm_ops = &paging_vma_ops;

    printk(KERN_INFO "paging_mmap() invoked: new VMA for pid %d from VA 0x%lx to 0x%lx\n",
    current->pid, vma->vm_start, vma->vm_end);

    if (demand_paging == 0) {
        // pre paging   
        struct page *pages = alloc_pages(GFP_KERNEL, my_get_order(nr_pages));
        if (pages == NULL) return -ENOMEM;
        atomic_add(nr_pages, &allocate_times);
        for (i = 0; i < nr_pages; i++) {
            state_ptr->pages[i] = pages+i;
            atomic_inc(&state_ptr->counter);
        }
        if (remap_pfn_range(vma, vma->vm_start, page_to_pfn(pages), PAGE_SIZE*nr_pages, vma->vm_page_prot) == 0) 
            return 0;
        else return -EFAULT;
    }

    return 0;
}

static struct file_operations
dev_ops =
{
    .mmap = paging_mmap,
};

static struct miscdevice
dev_handle =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = PAGING_MODULE_NAME,
    .fops = &dev_ops,
};
/*** END device I/O **/

/*** Kernel module initialization and teardown ***/
static int
kmod_paging_init(void)
{
    int status;

    /* Create a character device to communicate with user-space via file I/O operations */
    status = misc_register(&dev_handle);
    if (status != 0) {
        printk(KERN_ERR "Failed to register misc. device for module\n");
        return status;
    }

    printk(KERN_INFO "Loaded kmod_paging module\n");

    return 0;
}

static void
kmod_paging_exit(void)
{
    	/* Deregister our device file */
    	misc_deregister(&dev_handle);
        printk(KERN_INFO "The kernel has allocated %d pages, and freed %d pages\n", atomic_read(&allocate_times), atomic_read(&free_times)); // should be equal
    	printk(KERN_INFO "Unloaded kmod_paging module\n");
}

module_init(kmod_paging_init);
module_exit(kmod_paging_exit);

/* Misc module info */
MODULE_LICENSE("GPL");
