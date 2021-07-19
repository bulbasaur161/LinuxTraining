# Device tree path of board
- Device tree of am33xx SOC
```
arch/arm/boot/dts/am33xx.dtsi
```
- Device tree
arch/arm/boot/dts/am335x-evm.dts

# Device tree syntax
- Device node  
I2C
```
label: node-name@unit-address
unit-address is the first address of the reg property.
If device node doesn't contain any reg property, unit-address can be ommited.
Reg property explains the base address of the register of peripheral on the memory.
Example:
i2c0: i2c@44e0b000 {
			compatible = "ti,omap4-i2c";
			#address-cells = <1>;
			#size-cells = <0>;
			ti,hwmods = "i2c1";
			reg = <0x44e0b000 0x1000>;
			interrupts = <70>;
			status = "disabled";
		};
```
GPIO
```sh
#beagleboard/linux/blob/5.4/arch/arm/boot/dts/am33xx.dtsi
ocp: ocp {
		compatible = "simple-bus";
		#address-cells = <1>;
		#size-cells = <1>;
		ranges;
		ti,hwmods = "l3_main";

		l4_wkup: interconnect@44c00000 {
			wkup_m3: wkup_m3@100000 {
				compatible = "ti,am3352-wkup-m3";
				reg = <0x100000 0x4000>,
				      <0x180000 0x2000>;
				reg-names = "umem", "dmem";
				ti,hwmods = "wkup_m3";
				ti,pm-firmware = "am335x-pm-firmware.elf";
			};
		};
		
#beagleboard/linux/blob/5.4/arch/arm/boot/dts/am33xx-l4.dtsi
&l4_wkup {						/* 0x44c00000 */
	segment@200000 {					/* 0x44e00000 */
			compatible = "simple-bus";
			#address-cells = <1>;
			#size-cells = <1>;
			gpio0_target: target-module@7000 {	/* 0x44e07000, ap 14 20.0 */
						compatible = "ti,sysc-omap2", "ti,sysc";
						ti,hwmods = "gpio1";
						reg = <0x7000 0x4>,
						      <0x7010 0x4>,
						      <0x7114 0x4>;
```
- Build device tree
```sh
sudo make=ARM CROSS_COMPILE=arm-linux-gnueabihf- xxx.dtb
```

# Update u-boot support device tree
- U-boot v2019.04 support device tree  
https://source.denx.de/u-boot/u-boot/-/tree/v2019.04

- Build u-boot
```sh
make ARCH=arm am335x_evm_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
```
- Copy MLO, u-boot.img to board
- On the board, mount BOOT partion to /mnt/ folder and copy MLO, u-boot.img file to /mnt/
```sh
sudo -s
mount /dev/mmcblk0p
mount /dev/mmcblk0p1 /mnt/
```


