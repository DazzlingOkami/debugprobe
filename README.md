# Debugprobe with Black Magic

Firmware source for the Raspberry Pi Debug Probe SWD/UART accessory. Can also be run on a Raspberry Pi Pico.

[Raspberry Pi Debug Probe product page](https://www.raspberrypi.com/products/debug-probe/)

[Raspberry Pi Pico product page](https://www.raspberrypi.com/products/raspberry-pi-pico/)

[Black Magic home page](https://black-magic.org/)

# Documentation

Debug Probe documentation can be found in the [Pico Getting Started Guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf). See "Appendix A: Using the Debug Probe".

Black Magic Probe documentation can be found in the [Black Magic Probe Getting Started](https://black-magic.org/getting-started.html).

# Hacking

For the purpose of making changes or studying of the code, you may want to compile the code yourself.

First, clone the repository:
```
git clone https://github.com/DazzlingOkami/debugprobe
cd debugprobe
```
Initialize and update the submodules:
```
 git submodule update --init --depth=1
```
Then create and switch to the build directory:
```
 mkdir build
 cd build
```
If your environment doesn't contain `PICO_SDK_PATH`, then either add it to your environment variables with `export PICO_SDK_PATH=/path/to/sdk` or add `PICO_SDK_PATH=/path/to/sdk` to the arguments to CMake below.

Run cmake and build the code:
```
 cmake ..
 make
```
Done! You should now have a `debugprobe.uf2` that you can upload to your Debug Probe via the UF2 bootloader.

# Features
It support for BMP debug mode compared to the official firmware. It includes support for most targets, but only implements the SWD interface.

Refactored the code of the cdc_uart part and implemented a v2 version. It does not rely on the tud_cdc_connect() interface, which can make it more friendly to the host computer. I'm not sure if using DMA transfer would result in more efficient performance, but I did try it out. It achieves zero copy of data at the application layer.

Use floating point division mode for PIO to achieve more accurate SWD clock frequency.

Optimize the dual core of MCU by using SMP with FreeRTOS.

# TODO
1. BMP JTAG adapter support.
2. BMP run and error LED compatible with Debugprobe.
3. RTT support based on module within BMP.
