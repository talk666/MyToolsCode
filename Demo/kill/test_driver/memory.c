#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>/*printk*/
#include <linux/slab.h>/*kmalloc*/
#include <linux/fs.h>
#include <linux/errno.h>/*error code*/
#include <linux/types.h>/*size_t*/
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/switch_to.h>
#include <asm/uaccess.h>/*copy...*/


MODULE_LICENSE("Dual BSD/GPL");

static int memory_open(struct inode *inode, struct file *filp);
static int memory_release(struct inode *inode, struct file *filp);
static ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);



void memory_exit(void);
int memory_init(void);


module_init(memory_init);
module_exit(memory_exit);

/*access function*/
struct file_operations memory_fops = {
	.read = memory_read,
	.write = memory_write,
	.open = memory_open,
	.release = memory_release
};

/*Globle variables of the driver*/
int  memory_major = 60;
char *memory_buffer;



int memory_init(void)
{
	int result;
	
/*registering device*/
	result = register_chrdev(memory_major, "memory", &memory_fops);
	if(0 > result)
	{
		printk("<1>memory:cannot obtain major number %d\r\n", memory_major);
		return result;
	}

/*allocating memory for the buffer*/
	memory_buffer = kmalloc(1, GFP_KERNEL);//1Byte
	if(!memory_buffer)
	{
		result = -ENOMEM;
		goto fail;
	}
	memset(memory_buffer, 0, 1);
	
	printk("Inserting memory module\r\n");
	return 0;
fail:
	printk("<1>Inserting memory module faild\r\n");
	memory_exit();

	return result;

}


void memory_exit(void)
{

/*freeing the major number*/
	unregister_chrdev(memory_major, "memory");

/*free buff memory*/
	if(memory_buffer)
	{
		kfree(memory_buffer);
	}
	printk("<1>Removing memory module\r\n");
}


static int memory_open(struct inode *inode, struct file *filp)
{
	printk("<1>Open secceed!\r\n");
	return 0;

}

static int memory_release(struct inode *inode, struct file *filp)
{	
	printk("<1>Close secceed!\r\n");
	return 0;
}

static ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	printk("<1> RR secceed!\r\n");
	copy_to_user(buf, memory_buffer, 1);
	if(*f_pos == 0)
	{
		*f_pos += 1;
		return 1;
	}else{
		return 0;
	}
}

ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	printk("<1>WW secceed!\r\n");
	char *tmp;
	tmp = buf + count -1;
	copy_from_user(memory_buffer,tmp,1);
	return 1;
}
