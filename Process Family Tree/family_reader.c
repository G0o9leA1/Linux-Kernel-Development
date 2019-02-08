/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>

static int vpid;

/*
 * The "vpid" file where a static variable is read from and written to.
 */
static ssize_t vpid_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", vpid);
}

static ssize_t vpid_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;
	struct pid *ppid;
	ret = kstrtoint(buf, 10, &vpid);
	if (ret < 0)
		return ret;
	ppid = find_vpid(vpid);
	if (IS_ERR(ppid))
	{
		printk("Function find_vpid() failed!\n");
		return -1;
	}
	struct task_struct *task = pid_task(ppid, PIDTYPE_PID);
	if (IS_ERR(task))
	{
		printk("Function pid_task() failed!\n");
		return -1;
	}
	while (!IS_ERR(task))
	{
		printk("PID: %u, command name: %s\n", task->pid, task->comm);
		if (task->pid == 1)
			break;
		task = task->real_parent;
	}
	return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute vpid_attribute =
	__ATTR(vpid, 0664, vpid_show, vpid_store);

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&vpid_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *fam_reader_kobj;

static int __init example_init(void)
{
	int retval;

	/*
	 * Create a simple kobject with the name of "fam_reader",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	fam_reader_kobj = kobject_create_and_add("fam_reader", kernel_kobj);
	if (!fam_reader_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(fam_reader_kobj, &attr_group);
	if (retval)
		kobject_put(fam_reader_kobj);

	return retval;
}

static void __exit example_exit(void)
{
	kobject_put(fam_reader_kobj);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com> & GLW");
