# Sysfs
Sysfs  is mounted on the /sys/ folder.
```sh
cd /sys/
/sys# ls
```

The GPIO of the BBB is under the /class/gpio folder:
```sh
/sys/class/gpio# ls
```

Pin 8.8 is GPIO_67. To be able to control this pin, we need to export from the device tree:
```sh
/sys/class/gpio# echo 67 > export
/sys/class/gpio# ls
```

We are interested in making GPIO_67 as output. This is done by echoing "out" to the "direction" parameter.
```sh
root@arm:/sys/class/gpio/gpio67# echo out > direction
root@arm:/sys/class/gpio/gpio67# cat direction
out
```

We can now set GPIO_67 high or low by echoing "1" or "0" to the "value" parameter.
```sh
root@arm:/sys/class/gpio/gpio67# echo 1 > value
root@arm:/sys/class/gpio/gpio67# echo 0 > value
```

Delete.
```sh
root@arm:/sys/class/gpio# echo 67 > unexport
```

# Build program
```sh
g++ Blink_led.cpp -o blink
./blink
```
# Reference
https://www.teachmemicro.com/beaglebone-black-blink-led-using-c/  
https://www.teachmemicro.com/beaglebone-black-controlling-user-leds/  
https://www.youtube.com/watch?v=pJWcRPcqk3g  
http://derekmolloy.ie/kernel-gpio-programming-buttons-and-leds/
