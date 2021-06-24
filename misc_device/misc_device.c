#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ide.h>
#include <linux/device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/miscdevice.h>

#define MISC_DEVICE_NAME "misc_test"
#define MISC_DEVICE_MINOR 144

static int misc_open(struct inode *inode, struct file *filep)
{
	return 0;
}

static ssize_t misc_read(struct file *filp, char __user *buff, size_t cnt, loff_t *offt)
{
	return 0;
}

static ssize_t misc_write(struct file *filp, char __user *buff, size_t cnt, loff_t *offt)
{
	return 0;
}

static int misc_release(struct inode *inode, struct file *filep)
{
	return 0;
}

/* Operation function structure */
static struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.open = misc_open,
	.read = misc_read,
	.write = misc_write,
	.release = misc_release,
};

/* misc Equipment structure */
static struct miscdevice miscdevice_test = {
	.minor = MISC_DEVICE_MINOR,
	.name = MISC_DEVICE_NAME,
	.fops = &misc_fops,
};

static __init int misc_init(void)
{
	int ret = misc_register(&miscdevice_test);
	if (ret < 0)
		return -EFAULT;
	return 0;
}

static __exit void misc_exit(void)
{
	misc_deregister(&miscdevice_test);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_AUTHOR("eurphan<eurphan@163.com>");
MODULE_DESCRIPTION("Misc Driver");
MODULE_LICENSE("GPL");
