# LinuxTraining
```sh
~/kernel/kernel/hello/hello.c
#include        /* Needed by all modules */
#include        /* Needed for KERN_INFO */
#include          /* Needed for the macros */

static int __init hello_start(void)
{
    printk(KERN_INFO "Loading hello module...\n");
    printk(KERN_INFO "Hello world\n");
    return 0;
}

static void __exit hello_end(void)
{
    printk(KERN_INFO "Goodbye Mr.\n");
}

module_init(hello_start);
module_exit(hello_end);
 
~/kernel/kernel/hello/Makefile
obj-m := hello.o

PWD   := $(shell pwd)
KDIR  := ${PWD}/..

default:
    make -C $(KDIR) SUBDIRS=$(PWD) modules
 
~/kernel/kernel$ cd hello
~/kernel/kernel/hello$ make ARCH=arm CROSS_COMPILE=arm-angstrom-linux-gnueabi- 
```
