# I2C of BBB
i2c-0: 0x44E0_B000  no header  - HDMI TDA19988 (not accessible on header pin), EEPROM  
i2c-1: 0x4802_A000  header P9-17 P9-18  - free  
i2c-2: 0x4819_C000  header P9-19 P9-20  - reading EEPROMS(IC CAT24C256 - if use expansion board)  The EEPROMs on each expansion board are connected to I2C2 on connector P9 pins 19 and 20. For this reason I2C2 must always be left connected and should not be changed by SW to remove it from the expansion header pin mux settings. If this is done, then the system will be unable to detect the capes  
-> We wil connect ds3231 to I2C1 or I2C2 of BBB
- Offset I2C
```sh
#https://github.com/beagleboard/linux/blob/5.4/include/dt-bindings/pinctrl/am33xx.h
//I2C0
#define AM335X_PIN_I2C0_SDA			0x988 - ZCZ Pin Map C17
#define AM335X_PIN_I2C0_SCL			0x98c - ZCZ Pin Map C16
//I2C1
#define AM335X_PIN_SPI0_D1			0x958 - ZCZ Pin Map B16 - Header 9.18 - i2c1_sda
#define AM335X_PIN_SPI0_CS0			0x95c - ZCZ Pin Map A16 - Header 9.17 - i2c1_scl
//I2C2
#define AM335X_PIN_UART1_CTSN			0x978 - ZCZ Pin Map D18 - Header 9.20 - i2c2_sda
#define AM335X_PIN_UART1_RTSN			0x97c - ZCZ Pin Map D17 - Header 9.19 - i2c2_scl
```  
- Curent device tree
```sh
#https://github.com/beagleboard/linux/blob/5.4/arch/arm/boot/dts/am33xx.dtsi
aliases: aliases {
		i2c0 = &i2c0;
		i2c1 = &i2c1;
		i2c2 = &i2c2;
		
#https://github.com/beagleboard/linux/blob/5.4/arch/arm/boot/dts/am33xx-l4.dtsi
target-module@9c000 {			/* 0x4819c000, ap 46 5a.0 */
	compatible = "ti,sysc-omap2", "ti,sysc";
	ti,hwmods = "i2c3";
	reg = <0x9c000 0x8>,
	      <0x9c010 0x8>,
	      <0x9c090 0x8>;
	reg-names = "rev", "sysc", "syss";
	ti,sysc-mask = <(SYSC_OMAP2_CLOCKACTIVITY |
			 SYSC_OMAP2_ENAWAKEUP |
			 SYSC_OMAP2_SOFTRESET |
			 SYSC_OMAP2_AUTOIDLE)>;
	ti,sysc-sidle = <SYSC_IDLE_FORCE>,
			<SYSC_IDLE_NO>,
			<SYSC_IDLE_SMART>,
			<SYSC_IDLE_SMART_WKUP>;
	ti,syss-mask = <1>;
	/* Domains (P, C): per_pwrdm, l4ls_clkdm */
	clocks = <&l4ls_clkctrl AM3_L4LS_I2C3_CLKCTRL 0>;
	clock-names = "fck";
	#address-cells = <1>;
	#size-cells = <1>;
	ranges = <0x0 0x9c000 0x1000>;

	i2c2: i2c@0 {
		compatible = "ti,omap4-i2c";
		#address-cells = <1>;
		#size-cells = <0>;
		reg = <0x0 0x1000>;
		interrupts = <30>;
		status = "disabled";
	};
};

#https://github.com/beagleboard/linux/blob/5.4/arch/arm/boot/dts/am335x-bone-common.dtsi
&i2c2 {
	pinctrl-names = "default";
	//pinctrl-0 = <&i2c2_pins>;
	pinctrl-0 = <>;

	status = "okay";
	clock-frequency = <100000>;
	symlink = "bone/i2c/2";

	cape_eeprom0: cape_eeprom0@54 {
		compatible = "atmel,24c256";
		reg = <0x54>;
		#address-cells = <1>;
		#size-cells = <1>;
		cape0_data: cape_data@0 {
			reg = <0 0x100>;
		};
	};

	cape_eeprom1: cape_eeprom1@55 {
		compatible = "atmel,24c256";
		reg = <0x55>;
		#address-cells = <1>;
		#size-cells = <1>;
		cape1_data: cape_data@0 {
			reg = <0 0x100>;
		};
	};

	cape_eeprom2: cape_eeprom2@56 {
		compatible = "atmel,24c256";
		reg = <0x56>;
		#address-cells = <1>;
		#size-cells = <1>;
		cape2_data: cape_data@0 {
			reg = <0 0x100>;
		};
	};

	cape_eeprom3: cape_eeprom3@57 {
		compatible = "atmel,24c256";
		reg = <0x57>;
		#address-cells = <1>;
		#size-cells = <1>;
		cape3_data: cape_data@0 {
			reg = <0 0x100>;
		};
	};
};
```  
Check device tree of I2C2:
```sh
ls /sys/devices/platform/ocp/48000000.interconnect/48000000.interconnect:segment@100000/4819c000.target-module/4819c000.i2c/i2c-2
```
It will show: 54 - cape_eeprom0, 55 - cape_eeprom1, 56 - cape_eeprom2, 57 -cape_eeprom3
```sh
2-0054	2-0056	delete_device  i2c-dev	new_device  power      uevent
2-0055	2-0057	device	       name	of_node     subsystem
```
- Detect i2c
Connect ds3231 to i2c2 of BBB and run follow cmd to check i2c address
```sh
i2cdetect -r -y 2
```
I will show:
```sh
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- 57 -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- -- 
```
So the ds3231 's address is 0x68

# Read/Write DS3231 with I2C
Read:
- Send write command with register address to ds3231 's address in I2C bus.
```sh
	msg[0].addr  = 0x68;                  /* device address */
	msg[0].buf   = &address;              /* address of DS3231 register want to read */
	msg[0].len   = 1;                     /* 1 byte */
	msg[0].flags = 0;                     /* write */
```
- Send read command with number byte want to read to ds3231 's address in I2C bus.
```sh
	msg[1].addr  = 0x68;                  /* device address */
	msg[1].buf   = data;                  /* read buf */
	msg[1].len   = 3;                     /* 3 byte */
	msg[1].flags = I2C_M_RD;              /* read */
```
Write:
- Send write command with data to ds3231 's address in I2C bus.

# Reference
https://kipalog.com/posts/I2C-Client-Drivers-trong-Linux
