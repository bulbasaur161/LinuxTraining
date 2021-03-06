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
	dev_t dev_num; // For major and minor number
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

int bcd2dec(char num)
{
        return ((num/16 * 10) + (num % 16));
}

static ssize_t my_read(struct file *f, char *buf, size_t count, loff_t *off)
{
	struct i2c_device_data *dev = (struct i2c_device_data*)(f->private_data);
	struct i2c_adapter *adap = dev->client->adapter;
	struct i2c_msg msg[2];
	int ret;
	unsigned char data[4];
	unsigned char address;
	
	pr_info("Read device file\n");
	
	//if (count != 1)
	//	return -EINVAL;
	
	//temp = kmalloc(count, GFP_KERNEL);
	
	address = 0x00;

	msg[0].addr  = 0x68;                  /* device address */
	msg[0].buf   = &address;              /* address of DS3231 register want to read */
	msg[0].len   = 1;                     /* 1 byte */
	msg[0].flags = 0;                     /* write */

	/* Start read */
	msg[1].addr  = 0x68;                  /* device address */
	msg[1].buf   = data;                  /* read buf */
	msg[1].len   = 3;                     /* 3 byte */
	msg[1].flags = I2C_M_RD;              /* read */

	ret = i2c_transfer(adap, msg, 2);
	
	//if (ret >=0)
		//ret = copy_to_user(buf, &data, 1) ? -EFAULT : count;
	
	pr_info("The RTC time is %02d:%02d:%02d\n", bcd2dec(data[2]), bcd2dec(data[1]), bcd2dec(data[0]));

	address = 0x04;
	msg[1].len = 3;

	ret = i2c_transfer(adap, msg, 2);
	pr_info("The date is %02d/%02d/%02d\n", bcd2dec(data[0]), bcd2dec(data[1]), bcd2dec(data[2]));

	//address = 0x11;
	//msg[1].len = 2;

	//ret = i2c_transfer(adap, msg, 2);
	//float temperature = data[0] + ((data[1] >> 6) * 0.25);
	//pr_info("The temperature is %0.2fC\n", temperature);
	
	return 0;
}

static ssize_t my_write(struct file *f, const char *buf, size_t count, loff_t *off)
{
	pr_info("Write to device file\n");
	return 0;
}


static int my_open (struct inode *i, struct file *f)
{
	struct i2c_device_data *dev;
	
	pr_info("Open device file\n");
	dev = container_of(i->i_cdev, struct i2c_device_data, cdev);
	if(dev == NULL)
	{
		printk(KERN_ALERT" There is no data...\n");
		return -1;
	}
	f->private_data = dev;
	
	return 0;
}

static int my_close(struct inode *i, struct file *f)
{
	return 0;
}

struct file_operations fops = {
	.open		= my_open,
	.release 	= my_close,
	.read		= my_read,
	.write		= my_write,
};

/*Driver's private data */
struct i2c_driver_data ds3231_driver_data;

/* Probe Function to invoke the I2c Driver */
static int ds3231_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct i2c_device_data *data;
	struct device *dev = &client->dev;
	
	// Allocate i2c private data
	data = devm_kzalloc(&client->dev, sizeof(struct i2c_device_data), GFP_KERNEL);
	// Assign pointer to private data, it same dev_set_drvdata () func
	i2c_set_clientdata(client, data);
	data->client = client;
	
	/*4. Get the device number */
	data->dev_num = ds3231_driver_data.device_num_base + total_device;
	
	// Register entry point
	// Init cdev struct
	cdev_init(&data->cdev,&fops);
	data->cdev.owner = THIS_MODULE;
	// Register cdev struct to kernel
	// param1: &data->cdev - cdev struct
	// param2: data->dev_num - first device number of list device will be link with this cdev
	// param3: number of device will be link with this cdev
	ret = cdev_add(&data->cdev,data->dev_num,1);
	if(ret < 0){
		dev_err(dev,"Cdev add failed\n");
		return ret;
	}
	
	/*6. Create device file for the detected platform device */
	// param1: ds3231_driver_data.class - class create in /sys/class by class_create func
	// param2: dev = parrent device
	// param3: data->dev_num - device number of device
	// param4: NULL - additon data
	// param5: "i2cdev-%d" - device name, will be created in /dev
	ds3231_driver_data.device = device_create(ds3231_driver_data.class, dev, data->dev_num,NULL,\
								"i2cdev-%d",total_device);
	if(IS_ERR(ds3231_driver_data.device)){
		dev_err(dev,"Device create failed\n");
		ret = PTR_ERR(ds3231_driver_data.device);
		cdev_del(&data->cdev);
		return ret;
	}
	
	pr_info("module%d load success\n", total_device);
	total_device ++;
	return 0;
}

/* Remove Function */
static int ds3231_remove (struct i2c_client *client)
{
	struct i2c_device_data *device_data;
	
	// Assign pointer to private data, it same dev_get_drvdata  () func
	device_data = i2c_get_clientdata(client);
	
	/*1. Remove a device that was created with device_create() */
	device_destroy(ds3231_driver_data.class,device_data->dev_num);
	
	/*2. Remove a cdev entry from the system*/
	cdev_del(&device_data->cdev);
	
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
	pr_info("Init driver\n");

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
	pr_info("Remove driver\n");
	i2c_del_driver(&ds3231_I2C_drv);
	class_destroy(ds3231_driver_data.class);
	unregister_chrdev_region(ds3231_driver_data.device_num_base,MAX_DEVICES);
}

module_exit(i2c_client_drv_exit);
