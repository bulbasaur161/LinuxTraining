# 1. Install these packages on the HOST.
``` sh
sudo apt-get update
sudo apt-get install build-essential lzop u-boot-tools net-tools bison flex libssl-dev libncurses5-dev libncursesw5-dev unzip chrpath xz-utils minicom wget git-core
```
# 2. Download Debian 4G SD IOT.
https://beagleboard.org/latest-images

# 3. Tool-chain.
Download:  
- 32 bit:  
https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-i686_arm-linux-gnueabihf.tar.xz  
- 64 bit:  
-https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz  

Tool-chain PATH settings:
- Extract downloaded tool-chain, go to /bin folder of tool-chain and use pwd command to copy path.
- Go to you home directory.
- Open .bashrc using vim or gedit text editor.
- Copy the below export command with path information in the last line to .bashrc file:  
``` sh
export PATH=$PATH:<path_to_tool_chain_binaries>
```
  or simply do:  
``` sh
echo “export PATH=$PATH:<path_to_tool_chain_binaries>” > ~/.bashrc
```
- run command:
```sh
source .bashrc
```
- Type arm tab tab to check tool-chain installed.

# 4. Install GParted.
- Create boot partion: size 256 Mb, format fat16, name BOOT
- Create rootfs partion: size remain, format ext4, name ROOTFS
- After created partion, add some flag to boot partion. Right click boot partion -> Manage Flags -> select boot, lab flag -> close

# 5. Copy prebuilt image to SD card.
- Copy prebuit boot image to boot partion of SD card.
- Extract debian image. Right click debian image and mount this in file system.
- Go to mount point and copy all file to rootfs partion of SD card. (use command: sudo copy -a * /).

# 6. Kernel image, kernel modules update.
- Prebuilt kernel is 4.4.62, Clone the latest stable kernel source from BBB official github: https://github.com/beagleboard/linux . Clone 4.14 version.
- Build kernel:  
  Kernel source code compilation. This stage creates a kernel image "uImage" also all the device tree source files will be compiled, and dtbs will be generated.
```sh
cd <kernel source folder>
make ARCH=arm distclean
make ARCH=arm bb.org_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage dtbs LOADADDR=0x80008000 -j4
```
- Build kernel module:  
  This step builds and generates in-tree loadable(M) kernel modules(.ko). Installs all the generated .ko files in the default path of the host computer
(/lib/modules/<kernel_ver>). In case kernel_ver is 4.14.
```sh
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
sudo make ARCH=arm modules_install
```
- Copy uImage from host(../arch/arm/boot) to board and then update the boot partition of the SD card.
- Copy newly installed 4.14.108 folder from host to board's /lib/modules/ folder.
- Reset the board (you should see BBB boots with newly updated kernel image )

# 7. Makefile to build kernel modules
``` sh
#obj-<X>
#X = n, Do not compile the module
#X = y, Compile the module and link with kernel image
#X = m, Compile as dynamically loadable kernel module
obj-m := main.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
#set KERN_DIR to linux source location 
KERN_DIR = ~/workspace/ldd/source/linux_bbb_4.14/
HOST_KERN_DIR = /lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules
clean:
	make -C $(HOST_KERN_DIR) M=$(PWD) clean
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) help
host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
```
