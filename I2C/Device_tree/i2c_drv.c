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

#define MAX_DEVICES 10

static int total_device = 0;

/*Device private data structure */
struct i2c_device_data {
	struct i2c_client *client;
	dev_t dev; // For major and minor number
	u8 *buf;
	u16 value;
	struct cdev cdev;
};

/*Driver private data structure */
struct i2c_driver_data
{
	int total_devices;
	dev_t device_num_base; // For major and minor number
	struct class *class;
	struct device *device;
};

/*Driver's private data */
struct i2c_driver_data ds3231_driver_data;

/* Probe Function to invoke the I2c Driver */
static int ds3231_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_device_data *data;
	int result;
	
	// Allocate i2c private data
	data = devm_kzalloc(&client->dev, sizeof(struct i2c_device_data), GFP_KERNEL);
	// Assign pointer to private data, it same dev_set_drvdata () func
	i2c_set_clientdata(client, data);

	pr_info("module%d load success\n", total_device);
	total_device ++;
	return 0;
}

/* Remove Function */
static int ds3231_remove (struct i2c_client *client)
{
	struct i2c_device_data *data;
	
	// Assign pointer to private data, it same dev_get_drvdata  () func
	data = i2c_get_clientdata(client);
	
	class_destroy(ds3231_driver_data.class);
	unregister_chrdev_region(ds3231_driver_data.device_num_base,MAX_DEVICES);
	total_device --;
	pr_info("Remove module%d success\n", total_device);
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
	int ret;

	/*1. Dynamically allocate a device number for MAX_DEVICES */
	// Allocate device number: minor of first device 0, number of device allocated MAX_DEVICES
	// device_num_base pointer first dev_t return
	// character device name "i2c_drv_ds3231" will appear in /proc/devices
	ret = alloc_chrdev_region(&ds3231_driver_data.device_num_base,0,MAX_DEVICES,"i2c_drv_ds3231");
	if(ret < 0){
		pr_err("Alloc chrdev failed\n");
		return ret;
	}
	
	printk("Major Number = %d\n", MAJOR(ds3231_driver_data.device_num_base));
	
	/*2. Create device class under /sys/class */
	ds3231_driver_data.class = class_create(THIS_MODULE,"i2c_ds3231");
	if(IS_ERR(ds3231_driver_data.class)){
		pr_err("Class creation failed\n");
		ret = PTR_ERR(ds3231_driver_data.class);
		unregister_chrdev_region(ds3231_driver_data.device_num_base,MAX_DEVICES);
		return ret;
	}
	
	//3. Register with i2c-core 
	return i2c_add_driver(&ds3231_I2C_drv);
}

module_init(i2c_client_drv_init);

/* Exit module */
static void __exit i2c_client_drv_exit(void)
{
	i2c_del_driver(&ds3231_I2C_drv);
}

module_exit(i2c_client_drv_exit);
