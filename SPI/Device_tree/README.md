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
```
If you use offset address in spi device tree, you need use module_spi_driver, sample_probe(struct spi_device *spi) in kernel module.
