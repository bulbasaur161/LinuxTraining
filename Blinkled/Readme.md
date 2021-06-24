# Build program
```sh
g++ Blink_led.cpp -o blink
./blink
```

  If the target uses Device Tree mechanism like some embedded systems, Raspberry Pi for example.
  Its device tree may need to be updated first.
  There is a device tree overlay for Raspberry Pi in the dts-overlay folder for example.
  Just ``` make ``` in the folder, than it will compile and install the device tree overlay, and reboot is needed.
