#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("me");
MODULE_DESCRIPTION("Gpio test");
MODULE_VERSION("0.1");

static int __init gpio_init(void)
{
  pr_info("GPIO_TEST: Initializing the GPIO_TEST LKM\n");
	return 0;
}

static void __exit gpio_exit(void)
{
  pr_info("GPIO_TEST: End of the LKM!\n");
}

module_init(gpio_init);
module_exit(gpio_exit);
