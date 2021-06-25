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
- Type arm tab tab to check tool-chain installed.

# 4. Install GParted.
- Create boot partion: size 256 Mb, format fat16, name BOOT
- Create rootfs partion: size remain, format ext4, name ROOTFS
- After created partion, add some flag to boot partion. Right click boot partion -> Manage Flags -> select boot, lab flag -> close

# 5. Copy prebuilt image to SD card.
- Copy prebuit boot image to boot partion of SD card.
- Extract debian image. Right click debian image and mount this in file system.
- Go to mount point and copy all file to rootfs partion of SD card. (use command: sudo copy -a * /).

# 6. Kernel image update.
- Prebuilt kernel is 4.4.62
