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

struct omap2_mcspi mcspi;

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
	u32 l =0, div;
	u32 speed_hz = 500000;
	u8 word_len =8;
	ENTER();

	speed_hz = min_t(u32, speed_hz, OMAP2_MCSPI_MAX_FREQ);
	div = omap2_mcspi_calc_divisor(speed_hz);

	 l = mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0);

	// Select data line 0 for reception and data line 1 for transmission
	if (mcspi->pin_dir == MCSPI_PINDIR_D0_IN_D1_OUT) {
		l &= ~OMAP2_MCSPI_CHCONF_IS;
		l &= ~OMAP2_MCSPI_CHCONF_DPE1;
		l |= OMAP2_MCSPI_CHCONF_DPE0;
	} else {
		l |= OMAP2_MCSPI_CHCONF_IS;
		l |= OMAP2_MCSPI_CHCONF_DPE1;
		l &= ~OMAP2_MCSPI_CHCONF_DPE0;
	}

	// Set the word length as per word length
	l &= ~OMAP2_MCSPI_CHCONF_WL_MASK;
	l |= (word_len - 1) << 7;

	// Set the SPIEN state as high during active state
	l &= ~OMAP2_MCSPI_CHCONF_EPOL;

	//Set the clock divisor
	l &= ~OMAP2_MCSPI_CHCONF_CLKD_MASK;
	l |= div << 2;

	//set the PHA and POLso that the data is latched on odd numbers
	l &= ~OMAP2_MCSPI_CHCONF_PHA;
	l &= ~OMAP2_MCSPI_CHCONF_POL;
	
	//update the chconf0 register
	mcspi_write_chconf0(mcspi, l); 

	//force the chipselect
	omap2_mcspi_force_cs(mcspi, 1);

	return 0;

}

static void omap2_mcspi_set_enable(struct omap2_mcspi *mcspi, int enable)
{	
	u32 l;
	l = enable ? OMAP2_MCSPI_CHCTRL_EN : 0;
	mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCTRL0, l);
	/* Flash post-writes */
	mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCTRL0);
}

int spi_rw(struct omap2_mcspi *mcspi, char *buff)
{
	void __iomem		*base = mcspi->base;
	void __iomem		*tx_reg;
	void __iomem		*rx_reg;
	void __iomem		*chstat_reg;
	u8 rx[5];
	const u8 tx[5] = {0x50, 0x90, 0x80};
	u8 idx = 0;
	ENTER();
	
		/* We store the pre-calculated register addresses on stack to speed
	 * up the transfer loop. */
	tx_reg		= base + OMAP2_MCSPI_TX0;
	rx_reg		= base + OMAP2_MCSPI_RX0;
	chstat_reg	= base + OMAP2_MCSPI_CHSTAT0;

	// Enable the channel
	omap2_mcspi_set_enable(mcspi, 1);
	//Force the chipselect
	omap2_mcspi_force_cs(mcspi, 1);
	// Wait for TXS bit to be set
	if (mcspi_wait_for_reg_bit(chstat_reg,
						OMAP2_MCSPI_CHSTAT_TXS) < 0) {
					dev_err(mcspi->dev, "TXS timed out\n");
					return -1;
				}
	//Write into the tx_reg with __raw_writel
	__raw_writel(tx[idx], tx_reg);
	// Wait for RXS bit to be set
	if (mcspi_wait_for_reg_bit(chstat_reg,
						OMAP2_MCSPI_CHSTAT_RXS) < 0) {
					dev_err(mcspi->dev, "RXS timed out\n");
					return -1;
				}
	//Read the bytes of data into the rx_reg
	*rx = __raw_readl(rx_reg);
	printk("rx[%d] =%x\t",idx,rx[idx]);

	//Disable the cs force
	omap2_mcspi_force_cs(mcspi, 0);
	//Disable the channnel
	omap2_mcspi_set_enable(mcspi, 0);


	return 1;

}

static void omap2_mcspi_set_master_mode(struct omap2_mcspi *mcspi)
{
	ENTER();
	struct omap2_mcspi_regs	*ctx = &mcspi->ctx;
	u32 l;
	mcspi_write_reg(mcspi, OMAP2_MCSPI_WAKEUPENABLE, OMAP2_MCSPI_WAKEUPENABLE_WKEN);
	/*
	 * Set the single channel master mode and put the controller in functional mode
	 */
	l = mcspi_read_reg(mcspi, OMAP2_MCSPI_MODULCTRL);
	l &= ~(OMAP2_MCSPI_MODULCTRL_STEST | OMAP2_MCSPI_MODULCTRL_MS);
	l |= OMAP2_MCSPI_MODULCTRL_SINGLE;
	mcspi_write_reg(mcspi, OMAP2_MCSPI_MODULCTRL, l);
	ctx->modulctrl = l;

}

static int __init omap_spi_init_driver(void)
{
	void __iomem *spi_pad_base;

	// Get the virtual address for the spi0 base address and store it into base field of omap2_mcspi structure, Add the offset 0x100 to the base address.
	mcspi.base = ioremap(0x48030100, 0x1000);
	if(IS_ERR(mcspi.base)) 
	{
		printk(KERN_ERR"Unable to get the virtual address.\n");
		return PTR_ERR(mcspi.base);
	}
	// Set the pinmux for spi0
	spi_pad_base = ioremap(0x44E10950, 0x10);
	if(IS_ERR(spi_pad_base)) 
	{
		printk(KERN_ERR" Unabke to set the pinmux..\n");
		return PTR_ERR(spi_pad_base);
	}
	__raw_writel(0x30, spi_pad_base);
	__raw_writel(0x30, spi_pad_base + 0x4);
	__raw_writel(0x30, spi_pad_base + 0x8);
	__raw_writel(0x30, spi_pad_base + 0xc);


	omap2_mcspi_set_master_mode(&mcspi);
	
	// Initialize the character driver interface
	chrdev_init(&mcspi);		

}

static void __exit omap_spi_exit_driver(void)
{
	chrdev_exit();
}

module_init(omap_spi_init_driver);
module_exit(omap_spi_exit_driver);

MODULE_AUTHOR("TechoGenius Academy<techogenius7519@gmail.com");
MODULE_DESCRIPTION("Low level SPI driver");
MODULE_LICENSE("GPL");
