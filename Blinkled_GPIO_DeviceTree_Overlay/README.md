# Device tree overlay
- Build device tree overlay
```sh
dtc -@ -I dts -O dtb -o overlay.dtbo overlay.dts
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
load mmc 0:1 ${loadaddr} uImage
bootm ${loadaddr} - ${fdtaddr}
```

# Edit uEnv.txt file
```sh
console=ttyS0,115200n8
dtb=am335x-boneblack.dtb
dtbopath=/lib/firmware
ovenvsetup=setenv fdtaddr 0x87f00000;setenv fdtovaddr 0x87fc0000;
fdtload=load mmc 0:1 ${fdtaddr} ${dtb};
```
# Reference
http://www.righto.com/2016/08/the-beaglebones-io-pins-inside-software.html#ref2
