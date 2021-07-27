#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("--");
MODULE_DESCRIPTION("A I2C DS3231 driver");

static int total_device = 0;

/* Probe Function to invoke the I2c Driver */
static int ds3231_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_data *data;
	int result;

	pr_info("module%d load success\n", total_device);
	total_device ++;
	return 0;
}

/* Remove Function */
static int ds3231_remove (struct i2c_client *client)
{
	pr_info("Remove module%d success\n", total_device);
	total_device --;
	return 0;
}

static const struct i2c_device_id i2c_ids[] = {
	{"ds3231",0},
	{ }
};

MODULE_DEVICE_TABLE(i2c, i2c_ids);

static const struct of_device_id ds1672_of_match[] = {
	//{ .compatible = "dallas,ds3231" },
	{ .compatible = "atmel,24c256" },
	{ }
};

static struct i2c_driver ds3231_I2C_drv = {
	.driver = {
		.name = "ds3231",
		.of_match_table = of_match_ptr(ds1672_of_match),
	},
	.probe = ds3231_probe,
	.remove = ds3231_remove,
	.id_table = i2c_ids,
};

/* Initialization Module */
static int __init i2c_client_drv_init(void)
{
	// Register with i2c-core 
	return i2c_add_driver(&ds3231_I2C_drv);
}

module_init(i2c_client_drv_init);

/* Exit module */
static void __exit i2c_client_drv_exit(void)
{
	i2c_del_driver(&ds3231_I2C_drv);
}

module_exit(i2c_client_drv_exit);
