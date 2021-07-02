# Device tree path of board
- Device tree of am33xx SOC
```
arch/arm/boot/dts/am33xx.dtsi
```
- Device tree
arch/arm/boot/dts/am335x-evm.dts

# Device tree syntax
- Device node
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

# Device tree overlay
- Build device tree overlay
```sh
dtc -@ -I dts -O dtb -o overlay.dtbo overlay.dts
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

# Patch overlay to device tree
```sh
#load base blod
setenv fdtaddr 0x87f00000
#laod overlay blod
setenv fdtovaddr 0x87fc0000
#mmc0 is SD card, 1 is mmcblk0p1 (BOOT partion)
load mmc 0:1 ${fdtaddr} am335x-boneblack.dtb
#overlay.dtbo will load to 2 partion (ROOTFS partion)
load mmc 0:2 ${fdtovaddr} /lib/firmware/overlay.dtbo
#Set it as the working fdt tree
fdt addr $fdtaddr
#Resize if have multil overlay
fdt resize 8192
#apply overlay
fdt apply $fdtovaddr
```
