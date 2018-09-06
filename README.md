# Firmware

This project is about a miniaturization of the Vive Tracker by HTC.
It allows sub-millimetric 3d positioning, and embeds a 9DoF IMU with sensor
fusion. This project is open source, all the materials can be found online:

https://hivetracker.github.io

This repository contains the Arduino code to program it, more explanations to
use it are below:


## Install

The processor used in this project is not Arduino compatible by default but
the following steps explain how to do it anyway.

1) Download the Arduino IDE:

https://www.arduino.cc/en/Main/Software

2) Install the nRF52 support - read carefully these instructions:

https://github.com/sandeepmistry/arduino-nRF5#installing


## Compile

In the "Tools" menu select:
- "Board->Generic nRF52"
- "Softdevice->S132" (select "None" if you don't use BLE)
- The rest depends on your personal configuration.

If you prefer command line, you can also do:

    make compile

## Program

### Probes

This board doesn't come with a programmer for now but several cheap solutions
exist:

1) Adafruit sells J-Link and ST-Link probes:

https://www.adafruit.com/product/3571

https://www.adafruit.com/product/2548

2) For those with limited budget, these alternatives work too:

https://www.aliexpress.com/item/1PCS-ST-LINK-Stlink-ST-Link-V2-Mini-STM8-STM32-Simulator-Download-Programmer-Programming-With-Cover/32839270086.html

https://www.aliexpress.com/item/1PCS-Jlink-for-SWD-Jlink-3-Wire-for-STM32-on-SWD-Debug-New/32746364818.html

3) But if you already have a [Teensy](https://www.pjrc.com/teensy/) or a 3.3V
Arduino compatible board with ATmega32U4 (such as
[this one](https://www.aliexpress.com/item/Beetle-USB-ATMEGA32U4-Mini-Development-Board-Module-Leonardo-R3/32710925124.html)),
you can build your own programmer thanks to this repository:

https://github.com/myelin/arduino-cmsis-dap


### Setup

The above Install section should be enough in most cases but you might need to
install udev rule on linux:

https://raw.githubusercontent.com/arduino/OpenOCD/master/contrib/60-openocd.rules

Finally, in the "Tools" menu select the right SWD probe:
- "Programmer->J-Link"
- "Programmer->ST-Link"


### Upload

In the Arduino IDE it's the same as usual...

...and if you prefer command line, you can also do:

    make # upload is by default

## Licence
[GNU General Public License version 3](https://opensource.org/licenses/GPL-3.0)


## Enjoy!

