#include <linux/err.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include "spi_char.h"

struct sample_data {
	struct spi_device *spi;
	struct spi_message msg;
	struct spi_transfer transfer[2];
	u8 tx_buf;
	u8 rx_buf[2];
	// character device driver files
	dev_t devt;
	struct cdev cdev;
	struct class *class;
};


static ssize_t sample_read(struct file* f, char *buf, size_t count, loff_t *f_pos)
{
	unsigned char buf[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
	unsigned char recv[5];
	int res;
	struct sample_data *dev = (struct sample_data*) (f->private_data);
	
	struct spi_transfer tr = 
    	{
		.tx_buf	= &buf,
		.rx_buf = &recv,
		.len = 1,
	};
	res = spi_sync_transfer(dev->spi, &tr, 1);
	printk(KERN_INFO "Write Result %d value: %u %u %u %u %u\n", res, buf[0], buf[1], buf[2], buf[3], buf[4]);
	printk(KERN_INFO "Got Result %d value: %u %u %u %u %u\n", res, recv[0], recv[1], recv[2], recv[3], recv[4]);
	recv[0] = 0;
	
	return 0;
}

static int sample_close(struct inode *i, struct file *file)
{
	return 0;
}

static int sample_open(struct inode *i, struct file *f)
{
	struct sample_data *data = container_of(i->i_cdev, struct sample_data, cdev);
	if(data == NULL)
	{
		printk("data is null\n");
		return -1;
	}
	f->private_data = data;

	return 0;
}

struct file_operations fops = {
	.open = sample_open,
	.release = sample_close,
	.read = sample_read,
};

static int sample_probe(struct spi_device *spi)
{
	unsigned char buf[5] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
	unsigned char recv[5];
	struct sample_data *data;
	int init_result;
	int res;
	
	ENTER();

	data = devm_kzalloc(&spi->dev, sizeof(struct sample_data), GFP_KERNEL);
	data->spi = spi;
	spi_set_drvdata(spi,data);

	init_result = alloc_chrdev_region(&data->devt, 0, 1, "spi_smp");

	if (0 > init_result)
	{
		printk(KERN_ALERT "Device Registration failed\n");
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}
	printk("Major Nr: %d\n", MAJOR(data->devt));
	
	cdev_init(&data->cdev, &fops);

	// Register the file ops
	if (cdev_add(&data->cdev, data->devt, 1) == -1)
	{
		printk( KERN_ALERT "Device addition failed\n" );
		device_destroy(data->class, data->devt);
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1 );
		return -1;
	}

	if ((data->class = class_create(THIS_MODULE, "spisample")) == NULL)
	{
		printk( KERN_ALERT "Class creation failed\n" );
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}
	 //Create the device file with name spi_smp0
	if (device_create(data->class, NULL, data->devt, NULL, "spi_smp%d", 0) == NULL)
	{
		printk( KERN_ALERT "Device creation failed\n" );
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}
	
	struct spi_transfer tr = 
    	{
		.tx_buf	= &buf,
		.rx_buf = &recv,
		.len = 1,
	};
	spi_sync_transfer(spi, &tr, 1);
	
	//res = spi_write_then_read(spi, &buf, sizeof(buf), &recv, sizeof(recv));
	
	//res =  spi_write(spi, &buf, sizeof(buf));
	printk(KERN_INFO "Write Result %d value: %u %u %u %u %u\n", res, buf[0], buf[1], buf[2], buf[3], buf[4]);
	/* spi_read to read the data form our spi */
	mdelay(2);
	//res = spi_read(spi, &recv, sizeof(recv));
	printk(KERN_INFO "Got Result %d value: %u %u %u %u %u\n", res, recv[0], recv[1], recv[2], recv[3], recv[4]);
	recv[0] = 0;
	
	return 0;
}

static int sample_remove(struct spi_device *spi)
{
	struct sample_data *data = spi_get_drvdata(spi);

	// Delete the device file & the class
	device_destroy(data->class, data->devt);
   	
	//  Unregister file operations
	class_destroy(data->class);
	cdev_del(&data->cdev);
   	
	// Unregister character driver
	unregister_chrdev_region(data->devt, 1 );

	return 0;
}

//Populate the id table as per dtb
static const struct spi_device_id sample_id[] = {
	{ "my_spi", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, sample_id);

// Populate the spi_driver data structure
static struct spi_driver sample_driver = {
	.driver = {
		.name = "sample_client",
		.owner = THIS_MODULE,
	},
	.probe = sample_probe,
	.remove = sample_remove,
	.id_table = sample_id,
};
module_spi_driver(sample_driver);

MODULE_AUTHOR("TechoGenius Academy");
MODULE_DESCRIPTION("Sample Client driver");
MODULE_LICENSE("GPL v2");
