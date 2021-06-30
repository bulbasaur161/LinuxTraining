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
node-name@unit-address
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
