# Document of device tree  
https://www.kernel.org/doc/Documentation/gpio/board.txt
# Device tree path of board
- Device tree of am33xx SOC
```
arch/arm/boot/dts/am33xx.dtsi
```
- Device tree
arch/arm/boot/dts/am335x-evm.dts

# Device tree syntax  
Device tree is written in xml description language
- Device node  
I2C
```
label: node-name@unit-address
unit-address is the first address of the reg property.
If device node doesn't contain any reg property, unit-address can be ommited.
Reg property explains the base address of the register of peripheral on the memory.
Example:
#https://github.com/beagleboard/linux/blob/4.14/arch/arm/boot/dts/am33xx.dtsi
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

# Update kernel 5.4
- Kernel Beaglebone 5.4
https://github.com/beagleboard/linux/tree/5.4  
- Build kernel:
```sh
cd <kernel source folder>
make ARCH=arm distclean
make ARCH=arm bb.org_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage dtbs LOADADDR=0x80008000 -j4
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
sudo make ARCH=arm modules_install
```  
- Copy arch/arm/boot/dts/am335x-boneblack.dtb arch/arm/boot/uImage to BOOT partion.
- Copy modules 5.54 to ROOTFS partion:
```sh
cd /lib/modules/
sudo cp -a 5.4.106/ /ROOTFS/lib/modules/
sync
```

# Update device tree
- Create am335x-boneblack-gpio-test.dtsi  
  gpio 2.2 - GPIO66(2*32+2) - Header 8.7  
  gpio 2.3 - GPIO67(2*32+3) - Header 8.8  
- Edit am335x-boneblack.dts: include am335x-boneblack-gpio-test.dtsi  
- Compile device tree
```sh
cd <kernel source folder>
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- am335x-boneblack.dtb
```  
- Copy am335x-boneblack.dtb to boot partion.  
- Reboot board and check in /sys/devices/platform/ . It have bone_gpio_devs is node was create by device tree.
- Load gpio-sysfs.ko. Check whenever device added to /sys/class/bone-gpios.

# Test blink led
- Build gpio kernel module driver
- copy driver module to board
```sh
scp *.ko debian@192.168.7.2:/home/debian/drivers
```
- insmod driver
- use cat and echo to edit driver
```sh
cd /sys/class/bone_gpios/gpio2.3
#output 1 pin gpio2.3
#The redirection is done by the shell before sudo is even started. So either make sure the redirection happens in a shell with the right permissions
sudo bash -c 'echo 1 > value'
```

