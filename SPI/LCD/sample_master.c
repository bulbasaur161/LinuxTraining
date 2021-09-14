#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/omap-dma.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gcd.h>

#include <linux/spi/spi.h>

struct sample_master {
	struct spi_master *master;
	void __iomem *base;
	struct device *dev;
};


static int sample_spi_setup(struct spi_device *spi)
{
	printk("### sample setup invoked ###\n");

	return 0;
}

static void sample_spi_cleanup(struct spi_device *spi)
{
	printk("### sample clean up invoked ###\n");
}
static int sample_transfer_one_message(struct spi_master *master,
		struct spi_message *m)
{
	struct spi_transfer *t = NULL;
	int status =0;
	char *buf;
	printk(" #### Sample transfer one message is invoked..####\n");
	
	list_for_each_entry(t, &m->transfers, transfer_list) {
		if(t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			status = -EINVAL;
			break;
	}

	// Print the content of tx_buf and rx_buf
	if(t->tx_buf != NULL)
		printk("tx_buf = %d\n", *(u8 *) (t->tx_buf));
	if(t->rx_buf != NULL)
	{
		buf = t->rx_buf;
		buf[0] = 2;
		buf[1] = 4;
		t->len = 2;
	}
	}

	m->status = status;
	spi_finalize_current_message(master);
	return 0;
}

// Populate the device-id table. Compatible property should match with dtb
static const struct of_device_id sample_of_match[] = {
	{
		.compatible = "sample-master",
	},
	{ }, /* Null for termination */
};
MODULE_DEVICE_TABLE(of, sample_of_match);

static int sample_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master = NULL;
	struct sample_master *mcspi;
	struct device_node *node = pdev->dev.of_node;
	int status = 0;

	printk(" #### SPI Probe Function is invoked.#####\n");

	// Allocate the spi master with mcspi
	master = spi_alloc_master(&pdev->dev, sizeof(*mcspi));
	if(master == NULL) {
		printk(KERN_ALERT"Master allocation failed.\n");
		return -ENOMEM;
	}

	// spi->mode bits which is understood by the driver
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
	master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
	// Register the callback for setup
	master->setup = sample_spi_setup;
	// Register the callback handler for transfer_one_message
	master->transfer_one_message = sample_transfer_one_message;
	//Register the callback for cleanup
	master->cleanup = sample_spi_cleanup;

	master->dev.of_node = node;
	
	platform_set_drvdata(pdev, master);


	mcspi = spi_master_get_devdata(master);
	mcspi->master = master;
	// Register the spi master call
	status = spi_register_master(master);

	return status;
}

static int sample_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master;
	struct omap2_mcspi * mcspi;

	master = platform_get_drvdata(pdev);
	mcspi = spi_master_get_devdata(master);
	//Unregister the spi master
	spi_unregister_master(master);

	return 0;
}

//Populate the platform driver structure
static struct platform_driver sample_spi_driver = {
	.driver = {
		.name = "sample-spi",
		.owner = THIS_MODULE,
		.of_match_table = sample_of_match,
	},
	.probe = sample_spi_probe,
	.remove = sample_spi_remove,
};

module_platform_driver(sample_spi_driver);
MODULE_LICENSE("GPL");
