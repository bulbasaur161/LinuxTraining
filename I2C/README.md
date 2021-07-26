# I2C of BBB
i2c-0: 0x44E0_B000  header P9-17 P9-18  - not accessible on header pin  
i2c-1: 0x4802_A000  header P9-17 P9-18  - free  
i2c-2: 0x4819_C000  header P9-21 P9-22  - reading EEPROMS  
- Offset I2C
```sh
#https://github.com/beagleboard/linux/blob/5.4/include/dt-bindings/pinctrl/am33xx.h
//I2C0
#define AM335X_PIN_I2C0_SDA			0x988 - ZCZ Pin Map C17
#define AM335X_PIN_I2C0_SCL			0x98c - ZCZ Pin Map C16
//I2C1
#define AM335X_PIN_SPI0_D1			0x958 - ZCZ Pin Map B16
#define AM335X_PIN_SPI0_CS0			0x95c - ZCZ Pin Map A16
//I2C2
#define AM335X_PIN_UART1_CTSN			0x978 - ZCZ Pin Map D18
#define AM335X_PIN_UART1_RTSN			0x97c - ZCZ Pin Map D17
```
