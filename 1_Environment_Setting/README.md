# 1. Install these packages on the HOST
``` sh
sudo apt-get update
sudo apt-get install build-essential lzop u-boot-tools net-tools bison flex libssl-dev libncurses5-dev libncursesw5-dev unzip chrpath xz-utils minicom wget git-core
```
# 2. Download Debian 4G SD IOT
https://beagleboard.org/latest-images

# 3. Tool-chain.
Download:  
- 32 bit:  
https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-i686_arm-linux-gnueabihf.tar.xz  
- 64 bit:  
-https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz  

Tool-chain PATH settings:
- Extract downloaded tool-chain.
- Go to you home directory.
- Open .bashrc using vim or gedit text editor.
- Copy the below export command with path information to .bashrc file
      ``` sh
      export PATH=$PATH:<path_to_tool_chain_binaries>
      ```
  or simply do
      ``` sh
      echo “export PATH=$PATH:<path_to_tool_chain_binaries>” > ~/.bashrc
      ```

# 4. Install GParted
