#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("me");
MODULE_DESCRIPTION("Gpio test");
MODULE_VERSION("0.1");

#define PIN_NUMBER 4
static unsigned int gpioButton[PIN_NUMBER] = {67, 26, 46, 65};
struct timer_list led_timer;
static int status = 0;

static void blink_led(struct timer_list* timer);
static int dev_open(struct inode *, struct file *);
static int dev_close(struct inode *, struct file *);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int dev_ioctl(struct inode *i, struct file *f, unsigned int cmnd, unsigned long arg);
#else
static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
#endif

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,35))
	.ioctl = dev_ioctl
#else
	.unlocked_ioctl = dev_ioctl
#endif
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

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int dev_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
	switch(cmd)
	{
		case 1:
			pr_info("Start blink led\n");
			// Init timer
			init_timer(led_timer);
			// Set callback function of timer
			led_timer.function = blink_led; // Can use timer_setup(&led_timer, blink_led, 0);
			// Set timeout
			led_timer.expires = jiffies + HZ;
			// Start timer
			add_timer(&led_timer);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static void blink_led(struct timer_list* timer)
{
	if(status == 0)
	{
		gpio_set_value (67, 1);
		status = 1;
	}
	else
	{
		gpio_set_value (67, 0);
		status = 0;
	}
	
	// Set timeout
	led_timer.expires = jiffies + HZ;
	// Start timer
	add_timer(&led_timer);
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
