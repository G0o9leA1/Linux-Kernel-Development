/*
* Author: Chengyue Gong, Zixuan Li, Wentao Wang
* Orgnization: Washington University in St. Louis
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/fs_struct.h>
#include <linux/nsproxy.h>
#include <linux/dcache.h>
#include <linux/mount.h>
static struct task_struct *task;
static struct fs_struct *fs; // process's filesystem structure
static struct files_struct *files; // open file table structure
static struct nsproxy *nsproxy; // namespace proxy structure
static struct path pwd; // path structures for the process' current working directory
static struct path root; // path structures for the root directory
static struct vfsmount *root_mnt; // VFS mount structure for root path
static struct vfsmount *pwd_mnt; // VFS mount structure for pwd path
static struct dentry *root_dentry; // directory entry structure for root path
static struct dentry *pwd_dentry; // directory entry structure for pwd path

static void traverse_list(struct list_head *head) {
	struct dentry *each_entry;
	list_for_each_entry(each_entry,head,d_child)
	{
		printk(KERN_ALERT "%s\n",each_entry->d_iname);
		if(&(each_entry->d_subdirs)!=NULL)
		{
			traverse_list(&(each_entry->d_subdirs));
		}
	}
}

static int kthread_func(void *f)
{
	while(1)
	{
		fs = current->fs;
		printk(KERN_ALERT "fs address %p\n",fs);
		files = current->files;
		printk(KERN_ALERT "files address %p\n",files);
		nsproxy = current->nsproxy;
		printk(KERN_ALERT "nsproxy address %p\n",nsproxy);
		
		root=fs->root;
		pwd=fs->pwd;
		pwd_mnt=pwd.mnt;
		root_mnt=root.mnt;
		printk(KERN_ALERT "pwd_mnt %p\n",pwd_mnt);
		printk(KERN_ALERT "root_mnt %p\n",root_mnt);
		if(pwd_mnt!=root_mnt)
		{
			printk(KERN_ALERT "pwd_mnt_root %p\n",pwd_mnt->mnt_root);
			printk(KERN_ALERT "pwd_mnt_sb %p\n",pwd_mnt->mnt_sb);
			printk(KERN_ALERT "root_mnt_root %p\n",root_mnt->mnt_root);
			printk(KERN_ALERT "root_mnt_sb %p\n",root_mnt->mnt_sb);
		}
		
		pwd_dentry=pwd.dentry;
		root_dentry=root.dentry;
		printk(KERN_ALERT "pwd_dentry %p\n",pwd_dentry);
		printk(KERN_ALERT "root_dentry %p\n",root_dentry);
		if(pwd_dentry!=root_dentry)
		{
			printk(KERN_ALERT "root_dentry_diname %s\n",root_dentry->d_iname);
			printk(KERN_ALERT "pwd_dentry_diname %s\n",pwd_dentry->d_iname);
		}

		traverse_list(&(root_dentry->d_subdirs));
		
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if(kthread_should_stop())
			break;
	}
	return 0;
}

static int my_init(void)
{
	printk(KERN_ALERT "Module initialized\n");
	task = kthread_run(kthread_func,NULL,"single thread");
	return 0;
}

static void my_exit(void)
{
	if(!IS_ERR(task))
		kthread_stop(task);
	printk(KERN_ALERT "Module is being unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("GWL");
MODULE_DESCRIPTION ("422 Studio14");
