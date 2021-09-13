# SPI of BBB
- SPI0: CS0-P9.17, D0-P9.21, DI-P9.18, SCLK-P9.22 - free
- SPI1: CS0-P9.28, D0-P9.29, DI-P9.30, SCLK-P9.31 - TDA19988(HDMI)  
SPI1_SCLK(MCASP0_ACLKX), SPI1_CS0(MCASP0_AHCLKR), SPI1_D0(MCASP0_FSX) is used for HDMI. So we use SPI0 to connect LCD.
```sh
#https://github.com/beagleboard/linux/blob/5.4/arch/arm/boot/dts/am335x-boneblack-common.dtsi
mcasp0_pins: mcasp0_pins {
	pinctrl-single,pins = <
		AM33XX_PADCONF(AM335X_PIN_MCASP0_AHCLKX, PIN_INPUT_PULLUP, MUX_MODE0) /* mcasp0_ahcklx.mcasp0_ahclkx */
		AM33XX_PADCONF(AM335X_PIN_MCASP0_AHCLKR, PIN_OUTPUT_PULLDOWN, MUX_MODE2) /* mcasp0_ahclkr.mcasp0_axr2*/
		AM33XX_PADCONF(AM335X_PIN_MCASP0_FSX, PIN_OUTPUT_PULLUP, MUX_MODE0)
		AM33XX_PADCONF(AM335X_PIN_MCASP0_ACLKX, PIN_OUTPUT_PULLDOWN, MUX_MODE0)
		AM33XX_PADCONF(AM335X_PIN_GPMC_A11, PIN_OUTPUT_PULLDOWN, MUX_MODE7) /* gpmc_a11.GPIO1_27 */
	>;
};
	
&mcasp0	{
	#sound-dai-cells = <0>;
	pinctrl-names = "default";
	pinctrl-0 = <&mcasp0_pins>;
	status = "okay";
	op-mode = <0>;	/* MCASP_IIS_MODE */
	tdm-slots = <2>;
	serial-dir = <	/* 0: INACTIVE, 1: TX, 2: RX */
			0 0 1 0
		>;
	tx-num-evt = <32>;
	rx-num-evt = <32>;
};
```

# Pin connection
![image](https://user-images.githubusercontent.com/49242472/127280931-9d1e22f7-1cd3-4139-9c42-09035be82e70.png)

# Offset SPI
```sh
//SPI0
#define AM335X_PIN_SPI0_CS0			0x95c - ZCZ Pin Map A16 - Header 9.17 - CS0
#define AM335X_PIN_SPI0_D0			0x954 - ZCZ Pin Map B17 - Header 9.21 - D0
#define AM335X_PIN_SPI0_D1			0x958 - ZCZ Pin Map B16 - Header 9.18 - DI
#define AM335X_PIN_SPI0_SCLK			0x950 - ZCZ Pin Map A17 - Header 9.22 - SCLK
//SPI1
#define AM335X_PIN_MCASP0_AHCLKR		0x99c - ZCZ Pin Map C12 - Header 9.28 - CS0
#define AM335X_PIN_MCASP0_FSX			0x994 - ZCZ Pin Map B13 - Header 9.29 - D0
#define AM335X_PIN_MCASP0_AXR0			0x998 - ZCZ Pin Map D12 - Header 9.30 - DI
#define AM335X_PIN_MCASP0_ACLKX			0x990 - ZCZ Pin Map A13 - Header 9.31 - SCLK
```
# Device tree SPI
- Current device file
```sh
#https://github.com/beagleboard/linux/blob/5.4/arch/arm/boot/dts/am33xx.dtsi
spi0 = &spi0;
spi1 = &spi1;

#https://github.com/beagleboard/linux/blob/5.4/arch/arm/boot/dts/am33xx-l4.dtsi
target-module@30000 {			/* 0x48030000, ap 77 08.0 */
	compatible = "ti,sysc-omap2", "ti,sysc";
	ti,hwmods = "spi0";
	reg = <0x30000 0x4>,
	      <0x30110 0x4>,
	      <0x30114 0x4>;
	reg-names = "rev", "sysc", "syss";
	ti,sysc-mask = <(SYSC_OMAP2_CLOCKACTIVITY |
			 SYSC_OMAP2_SOFTRESET |
			 SYSC_OMAP2_AUTOIDLE)>;
	ti,sysc-sidle = <SYSC_IDLE_FORCE>,
			<SYSC_IDLE_NO>,
			<SYSC_IDLE_SMART>;
	ti,syss-mask = <1>;
	/* Domains (P, C): per_pwrdm, l4ls_clkdm */
	clocks = <&l4ls_clkctrl AM3_L4LS_SPI0_CLKCTRL 0>;
	clock-names = "fck";
	#address-cells = <1>;
	#size-cells = <1>;
	ranges = <0x0 0x30000 0x1000>;

	spi0: spi@0 {
		compatible = "ti,omap4-mcspi";
		#address-cells = <1>;
		#size-cells = <0>;
		reg = <0x0 0x400>;
		interrupts = <65>;
		ti,spi-num-cs = <2>;
		dmas = <&edma 16 0
			&edma 17 0
			&edma 18 0
			&edma 19 0>;
		dma-names = "tx0", "rx0", "tx1", "rx1";
		status = "disabled";
	};
};

```

# Reference
https://github.com/eziya/STM32_HAL_ILI9341  
https://m.blog.naver.com/jjong_w/220971986593  
http://matthewcmcmillan.blogspot.com/2014/09/experimenting-with-beaglebone-black-and-tft.html  
https://cdn-learn.adafruit.com/downloads/pdf/introduction-to-the-beaglebone-black-device-tree.pdf  
https://github.com/notro/fbtft/wiki  
http://papermint-designs.com/dmo-blog/2017-02-beaglebone-black-and-watterott-display  
https://embetronicx.com/tutorials/linux/device-drivers/linux-kernel-spi-device-driver-tutorial/
https://www.kernel.org/doc/Documentation/spi/spi-summary
