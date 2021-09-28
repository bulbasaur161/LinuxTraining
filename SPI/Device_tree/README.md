# Modify kernel
If use directly address access you need modify kernel for hardware mode to not to go to IDLE state.  
```sh
--- a/arch/arm/mach-omap2/omap_hwmod_33xx_43xx_ipblock_data.c
+++ b/arch/arm/mach-omap2/omap_hwmod_33xx_43xx_ipblock_data.c
@@ -886,6 +886,7 @@ struct omap_hwmod am33xx_spi0_hwmod = {
  .name = "spi0",
  .class = &am33xx_spi_hwmod_class,
  .clkdm_name = "l4ls_clkdm",
+ .flags = HWMOD_NO_IDLE,
  .main_clk = "dpll_per_m2_div4_ck",
  .prcm = {
  .omap4 = {
@@ -899,6 +900,7 @@ struct omap_hwmod am33xx_spi1_hwmod = {
  .name = "spi1",
  .class = &am33xx_spi_hwmod_class,
  .clkdm_name = "l4ls_clkdm",
+ .flags = HWMOD_NO_IDLE,
  .main_clk = "dpll_per_m2_div4_ck",
  .prcm = {
  .omap4 = {
```
# Kernel module
If you use direct spi address in device tree, you need use platform_driver_register, my_mcspi_probe(struct platform_device *pdev) in kernel module because it is considered as a platform device.
```sh
#arch/arm/boot/dts/am335x-boneblack.dts
my_spi: spi@0x48030000 {
		compatible = "my_spi";
		#address-cells = <1>;
		#size-cells = <0>;
		reg = <0x48030100 0x400>;
		pinctrl-names = "default";
		pinctrl-0 = <&spi0_pins>;
		status = "okay";
	};
	
&am33xx_pinmux {
	pinctrl-names = "default";
	pinctrl-0 = <&clkout2_pin>;
	
	spi0_pins: spi0_pins {
		pinctrl-single,pins = <
			AM33XX_PADCONF(AM335X_PIN_SPI0_SCLK, PIN_INPUT_PULLUP, MUX_MODE0)	/* P9.22 SCLK - pin13 Arduino SCLK*/
			AM33XX_PADCONF(AM335X_PIN_SPI0_D0, PIN_INPUT_PULLUP, MUX_MODE0)		/* P9.21 MISO - pin12 Arduino MISO*/
			AM33XX_PADCONF(AM335X_PIN_SPI0_D1, PIN_OUTPUT_PULLUP, MUX_MODE0)	/* P9.18 MOSI - pin11 Arduino MOSI*/
			AM33XX_PADCONF(AM335X_PIN_SPI0_CS0, PIN_OUTPUT_PULLUP, MUX_MODE0)	/* P9.17 CS0 - pin10 Arduino SS*/
		>;
	};
};
```
If you use offset address in spi device tree, you need use module_spi_driver, sample_probe(struct spi_device *spi) in kernel module.
```sh
#arch/arm/boot/dts/am335x-bone-common.dtsi
&am33xx_pinmux {
        spi0_pins: spi0_pins {
                pinctrl-single,pins = <
                        AM33XX_PADCONF(AM335X_PIN_SPI0_SCLK, PIN_INPUT_PULLUP, MUX_MODE0)       /* P9.22 */
                        AM33XX_PADCONF(AM335X_PIN_SPI0_D0, PIN_INPUT_PULLUP, MUX_MODE0)         /* P9.21 */
                        AM33XX_PADCONF(AM335X_PIN_SPI0_D1, PIN_OUTPUT_PULLUP, MUX_MODE0)        /* P9.18 */
                        AM33XX_PADCONF(AM335X_PIN_SPI0_CS0, PIN_OUTPUT_PULLUP, MUX_MODE0)       /* P9.17 */
                >;
        };
};

&spi0 {
        pinctrl-names = "default";
        pinctrl-0 = <&spi0_pins>;
        status = "okay";

        myslave0: my_spi0@0 {
                compatible = "my_spi";
                reg = <0>;
                spi-max-frequency = <10000000>;
        };
};
```
# Touch screen
If you use touch screen, SPI0 will be used for touch, SPI1 will be used for display.
spi1 and HDMI both uses P9.29 so HDMI (mcasp0_fsx) has to be disabled for spi1 (spi1_d0 MISO) to be used. This means that displays with a SPI LCD controller and a SPI touch controller can't have HDMI enabled if one SPI bus is used for both lcd and touch.
Custom displays that use both SPI busses can have HDMI enabled if spi0 is used for touch (has MISO) and spi1 is used for lcd.
```sh
&am33xx_pinmux {
        spi0_pins: spi0_pins {
                pinctrl-single,pins = <
                        AM33XX_PADCONF(AM335X_PIN_SPI0_SCLK, PIN_INPUT_PULLUP, MUX_MODE0)       /* P9.22 */
                        AM33XX_PADCONF(AM335X_PIN_SPI0_D0, PIN_INPUT_PULLUP, MUX_MODE0)         /* P9.21 */
                        AM33XX_PADCONF(AM335X_PIN_SPI0_D1, PIN_OUTPUT_PULLUP, MUX_MODE0)        /* P9.18 */
                        AM33XX_PADCONF(AM335X_PIN_SPI0_CS0, PIN_OUTPUT_PULLUP, MUX_MODE0)       /* P9.17 */
                >;
        };

        spi1_pins: spi1_pins {
                pinctrl-single,pins = <
                        AM33XX_PADCONF(AM335X_PIN_MCASP0_ACLKX, PIN_INPUT_PULLUP, MUX_MODE3)    /* P9.31 SCLK */
                       // AM33XX_PADCONF(AM335X_PIN_MCASP0_FSX, PIN_INPUT_PULLUP, MUX_MODE3)    /* P9.29 D0 */
                        AM33XX_PADCONF(AM335X_PIN_MCASP0_AXR0, PIN_OUTPUT_PULLUP, MUX_MODE3)    /* P9.30 D1 */
                        AM33XX_PADCONF(AM335X_PIN_MCASP0_AHCLKR, PIN_OUTPUT_PULLUP, MUX_MODE3)  /* P9.28 CS0 */
                >;
        };
}

&spi0 {
        pinctrl-names = "default";
        pinctrl-0 = <&spi0_pins>;
        status = "okay";

        myslave0: my_spi0@0 {
                compatible = "my_spi0";
                reg = <0>;
                spi-max-frequency = <10000000>;
        };
};

&spi1 {
        pinctrl-names = "default";
        pinctrl-0 = <&spi1_pins>;
        status = "okay";

        myslave1: my_spi0@0 {
                compatible = "my_spi1";
                reg = <0>;
                spi-max-frequency = <10000000>;
        };
};

```
