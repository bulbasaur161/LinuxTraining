#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("me");
MODULE_DESCRIPTION("Gpio test");
MODULE_VERSION("0.1");

#define PIN_NUMBER 4
static unsigned int gpioButton[PIN_NUMBER] = {44, 26, 46, 65};

static int dev_open(struct inode *, struct file *);
static int dev_close(struct inode *, struct file *);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
};

static struct miscdevice btn_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "buttons",
	.fops = &fops,
};

static int dev_open(struct inode *inodep, struct file *filep)
{
	return 0;
}

static int dev_close(struct inode *inodep, struct file *filep)
{
	/* Do Nothing */
	return 0;
}

static int __init gpio_init(void)
{
	int i=0;
	int ret, result;
	
	// Check invalid gpio
	for (i = 0; i< PIN_NUMBER; i++)
	{
		if (!gpio_is_valid(gpioButton[i]))
		{
			pr_info("This gpio pin %d is not valid\n", gpioButton[i]);
			return -1;
		}
	}
	
	// Request
	for (i = 0; i< PIN_NUMBER; i++)
	{
		ret = gpio_request(gpioButton[i], "sysfs");
		if (ret < 0)
		{
			pr_info("Failed to request GPIO\n");
			return -1;
		}
		//config pin is output
		gpio_direction_output(gpioButton[i], 0);
	}

	result = misc_register(&btn_dev);
	if (result) {
		pr_info("can't not register device\n");
		return result;
	}
	
 	pr_info("GPIO_TEST: Initializing the GPIO_TEST LKM\n");
	return 0;
}

static void __exit gpio_exit(void)
{
	int i;
	for (i = 0; i< PIN_NUMBER; i++)
	{
		gpio_free(gpioButton[i]);
	}

	misc_deregister(&btn_dev);
 	pr_info("GPIO_TEST: End of the LKM!\n");
}

module_init(gpio_init);
module_exit(gpio_exit);
