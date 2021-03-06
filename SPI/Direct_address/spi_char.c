#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/platform_data/serial-omap.h>
#include "spi_char.h"

#define FIRST_MINOR 0
#define MINOR_CNT 1

static struct omap2_mcspi *mcspi;

static int my_open(struct inode *i, struct file *f)
{
	return omap2_mcspi_setup_transfer(mcspi, NULL);
}
static int my_close(struct inode *i, struct file *f)
{
	return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	ENTER();
	uint8_t tx[] = {0x01, 0x80, 0x00};
	spi_rw(mcspi, tx);
	
	return 0;
}

static struct file_operations driver_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
};

int chrdev_init(struct omap2_mcspi *lmcspi)
{
	int ret =0;
	struct device *dev_ret = NULL;
	mcspi = lmcspi;

	if((ret = alloc_chrdev_region(&mcspi->devt, FIRST_MINOR, MINOR_CNT, "spi_driver")) <0)
	{
		return ret;
	}
	
	// Register the file operations
	cdev_init(&mcspi->cdev, &driver_fops);
	if((ret = cdev_add(&mcspi->cdev, mcspi->devt, MINOR_CNT)) < 0)
	{
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return ret;
	}

	if (ret < 0)
	{
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return ret;
	}

	// Creation of Device file with name spi0
	if(IS_ERR(lmcspi->spi_class = class_create(THIS_MODULE, "spi")))
	{
		cdev_del(&mcspi->cdev);
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return PTR_ERR(lmcspi->spi_class);	
	}

	if(IS_ERR(dev_ret = device_create(mcspi->spi_class, NULL, mcspi->devt, NULL, "spi%d", FIRST_MINOR)))
	{
		class_destroy(mcspi->spi_class);
		cdev_del(&mcspi->cdev);
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return PTR_ERR(dev_ret);
	}	
	
	if (IS_ERR(dev_ret))
	{
		class_destroy(mcspi->spi_class);
		cdev_del(&mcspi->cdev);
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return PTR_ERR(dev_ret);
	}
	return 0;
}

void chrdev_exit(void)
{
	// Delete the device file and class, unregister the file operations and character driver
		device_destroy(mcspi->spi_class, mcspi->devt);
		class_destroy(mcspi->spi_class);
		cdev_del(&mcspi->cdev);
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);

}
