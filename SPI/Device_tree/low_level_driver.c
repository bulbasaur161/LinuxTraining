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
#include "spi_char.h"

#include <linux/spi/spi.h>
#include <linux/platform_data/spi-omap2-mcspi.h>
#include "low_level_driver.h"

static inline void mcspi_write_reg(struct omap2_mcspi *mcspi,
        int idx, u32 val)
{
    __raw_writel(val, mcspi->base + idx);
}

static inline u32 mcspi_read_reg(struct omap2_mcspi *mcspi, int idx)
{
    return __raw_readl(mcspi->base + idx);
}

static inline void mcspi_write_chconf0(struct omap2_mcspi *mcspi, u32 val)
{
    mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCONF0, val);
    mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0);
}

static int mcspi_wait_for_reg_bit(void __iomem *reg, unsigned long bit)
{
    unsigned long timeout;

    timeout = jiffies + msecs_to_jiffies(1000);
    while (!(__raw_readl(reg) & bit)) {
        if (time_after(jiffies, timeout)) {
            if (!(__raw_readl(reg) & bit))
                return -ETIMEDOUT;
            else
                return 0;
        }
        cpu_relax();
    }
    return 0;
}

static void omap2_mcspi_force_cs(struct omap2_mcspi *mcspi, int cs_active)
{
    u32 l;
    ENTER();

    l = mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0);
    if (cs_active)
        l |= OMAP2_MCSPI_CHCONF_FORCE;
    else
        l &= ~OMAP2_MCSPI_CHCONF_FORCE;

    mcspi_write_chconf0(mcspi, l);
}

static u32 omap2_mcspi_calc_divisor(u32 speed_hz)
{
    u32 div;
    ENTER();

    for (div = 0; div < 15; div++)
        if (speed_hz >= (OMAP2_MCSPI_MAX_FREQ >> div))
            return div;
    return 15;
}

int omap2_mcspi_setup_transfer(struct omap2_mcspi *mcspi,
		struct spi_transfer *t) 
{
	u32 l = 0, div = 0;
    u32 speed_hz = 500000;
    u8 word_len = 8;
	ENTER();

    speed_hz = min_t(u32, speed_hz, OMAP2_MCSPI_MAX_FREQ);
    div = omap2_mcspi_calc_divisor(speed_hz);

	l = mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0); 
	// Select Data line 0 from reception & Data line 1 for transmission 
	if (mcspi->pin_dir == MCSPI_PINDIR_D0_IN_D1_OUT) {
		l &= ~OMAP2_MCSPI_CHCONF_IS;
		l &= ~OMAP2_MCSPI_CHCONF_DPE1;
		l |= OMAP2_MCSPI_CHCONF_DPE0;
	} else {
		l |= OMAP2_MCSPI_CHCONF_IS;
		l |= OMAP2_MCSPI_CHCONF_DPE1;
		l &= ~OMAP2_MCSPI_CHCONF_DPE0;
	}
	//  Set the word len as per word_len
	l &= ~OMAP2_MCSPI_CHCONF_WL_MASK;
	l |= (word_len - 1) << 7;

	//  Set the SPIEN state as high during active state
	l &= ~OMAP2_MCSPI_CHCONF_EPOL;
	/* set clock divisor */
	// Set the clock divider
	l &= ~OMAP2_MCSPI_CHCONF_CLKD_MASK;
	l |= div << 2;
	//  Set the PHA & POL so that the data is latched on odd numbered edges
	l &= ~OMAP2_MCSPI_CHCONF_PHA;
	l &= ~OMAP2_MCSPI_CHCONF_POL;
	//  Update the chconf0 register
	mcspi_write_chconf0(mcspi, l);
	omap2_mcspi_force_cs(mcspi, 1);
	return 0;
}

static void omap2_mcspi_set_enable(struct omap2_mcspi *mcspi, int enable)
{
    u32 l;
    ENTER();

    l = enable ? OMAP2_MCSPI_CHCTRL_EN : 0;
    mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCTRL0, l);
    /* Flash post-writes */
    mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCTRL0);
}

int spi_rw(struct omap2_mcspi *mcspi, char *buff)
{
	void __iomem        *base = mcspi->base;
    void __iomem        *tx_reg;
    void __iomem        *rx_reg;
    void __iomem        *chstat_reg;
    u8      rx[5];
    const u8    tx[5] = {0x50, 0x80, 0x60};
    u8 idx = 0;

	ENTER();

    /* We store the pre-calculated register addresses on stack to speed
     * up the transfer loop. */
    tx_reg = base + OMAP2_MCSPI_TX0;
    rx_reg = base + OMAP2_MCSPI_RX0;
    chstat_reg = base + OMAP2_MCSPI_CHSTAT0;

	// Enable the channel
	omap2_mcspi_set_enable(mcspi, 1);
	// Force the Chipselect
	omap2_mcspi_force_cs(mcspi, 1);
	//  Wait for TXS bit to be set
	if (mcspi_wait_for_reg_bit(chstat_reg,
				OMAP2_MCSPI_CHSTAT_TXS) < 0) {
		dev_err(mcspi->dev, "TXS timed out\n");
		return -1;
	}
	//  Write into the tx_reg with __raw_writel
	__raw_writel(tx[idx], tx_reg);
	// Wait for RXS bit to be set
	if (mcspi_wait_for_reg_bit(chstat_reg,
				OMAP2_MCSPI_CHSTAT_RXS) < 0) {
		dev_err(mcspi->dev, "RXS timed out\n");
		return -1;
	}
	// Read a bytes of data into the rx buffer
	*rx = __raw_readl(rx_reg);
	printk("rx[%d] = %x\t", idx, rx[idx]);
	// Disable the cs force
	omap2_mcspi_force_cs(mcspi, 0);
	// Disable the channel
	omap2_mcspi_set_enable(mcspi, 0);
	return 1;
}

static void omap2_mcspi_set_master_mode(struct omap2_mcspi *mcspi)
{
    u32 l;
	struct omap2_mcspi_regs	*ctx = &mcspi->ctx;
    ENTER();
	
	mcspi_write_reg(mcspi, OMAP2_MCSPI_WAKEUPENABLE,
            OMAP2_MCSPI_WAKEUPENABLE_WKEN);
    l = mcspi_read_reg(mcspi, OMAP2_MCSPI_MODULCTRL);
	// Set single channel master mode & put the controller in functional mode 
	l &= ~(OMAP2_MCSPI_MODULCTRL_STEST | OMAP2_MCSPI_MODULCTRL_MS);
	l |= OMAP2_MCSPI_MODULCTRL_SINGLE;
	mcspi_write_reg(mcspi, OMAP2_MCSPI_MODULCTRL, l);

	ctx->modulctrl = l;
}

static int my_mcspi_probe(struct platform_device *pdev)
{
    struct omap2_mcspi *mcspi;
    struct resource *r = NULL;

	//Allocate the memory for omap2_mcspi and assign it into mcspi
	mcspi = kzalloc(sizeof(*mcspi), GFP_KERNEL);
	platform_set_drvdata(pdev, mcspi);

	//Get the base address from the platform device
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(r == NULL)
	{
		return -ENODEV;
	}

	//Get the virtual address for spi0 base address and assign it into the base field of mcspi.
	mcspi->base = ioremap(r->start, resource_size(r));
	if(IS_ERR(mcspi->base)) {
		printk(KERN_ERR"Unable to get the virtual address.\n");
		return PTR_ERR(mcspi->base);
	}

	mcspi->dev = &pdev->dev;
	

	omap2_mcspi_set_master_mode(mcspi);

    // Initialize the character driver interface
	chrdev_init(mcspi);
    return 0;
}

static int my_mcspi_remove(struct platform_device *pdev)
{
	struct omap2_mcspi *mcspi;
    //De-initialize the character driver interface
	chrdev_exit();

	mcspi = platform_get_drvdata(pdev);
	// free the mcspi data structure
	kfree(mcspi);

	return 0;
}

//Populate the id table with compatible property as per dtb
static const struct of_device_id my_mcspi_of_match[] = {
    {
	.compatible = "my_spi",
    },
    { }, /* NULL for termination */
};

// Populate the platform driver structure
static struct platform_driver my_mcspi_driver = {
	.driver = {
		.name 	= "omap2_mcspi2",
		.owner	= THIS_MODULE,
	.of_match_table	= my_mcspi_of_match,
	},
	.probe 	= my_mcspi_probe,
	.remove = my_mcspi_remove,
};

static int __init omap_spi_init_driver(void)
{
	//Register the Platform Driver
	return platform_driver_register(&my_mcspi_driver);
}

static void __exit omap_spi_exit_driver(void)
{
	//De-register the platform driver
	return platform_driver_unregister(&my_mcspi_driver);
}
module_init(omap_spi_init_driver);
module_exit(omap_spi_exit_driver);

MODULE_AUTHOR("TechoGenius Academy<techogenius7519@gmail.com>");
MODULE_DESCRIPTION("Low level SPI driver");
MODULE_LICENSE("GPL");
