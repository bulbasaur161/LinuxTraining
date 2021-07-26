# I2C of BBB
i2c-0: 0x44E0_B000  no header  - HDMI TDA19988 (not accessible on header pin), EEPROM  
i2c-1: 0x4802_A000  header P9-17 P9-18  - free  
i2c-2: 0x4819_C000  header P9-19 P9-20  - reading EEPROMS(IC CAT24C256 - if use expansion board)  The EEPROMs on each expansion board are connected to I2C2 on connector P9 pins 19 and 20. For this reason I2C2 must always be left connected and should not be changed by SW to remove it from the expansion header pin mux settings. If this is done, then the system will be unable to detect the capes
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
- Detect i2c
```sh
i2cdetect -r -y 2
```
