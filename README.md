# Automatic monitor brightness controller
This is an automatic "*monitor brightness controller*" based on environmental light conditions. This system use USB port base sensor unit to measure the light level and control monitor brightness accordingly. We design this system to reduce the eye stress by matching the monitor brightness with environmental lighting. 

The sensor unit of this system is build around PIC18F2550 8-bit microcontroller. To measure the light level we use LDR with MCU's inbuilt ADC. The control software of this unit is design to work with *Microsoft Windows* operating systems and it use Windows API's DDC/CI related functions to control the monitors/display devices.

Firmware of USB sensor use 50-point boxcar filter to get the average readout from the ADC and that value is passed to PC over USB HID interface. Thanks to this HID interface this system may not need any special device driver, and it can use in any compatible PC with minimum amount of configuration changes. 

This project package contain two console applications. The first application (*sensor-logger*) is use to monitor the sensor readings and we design it to calibrate the control software. 

The control software is named as *brightness-control*. Both these software may not need any command line arguments and they can launch by just double clicking or through the command line console. 

All the PC applications of this project are developed using *Visual C++ 2012* and PIC 18F2550 firmware is developed using *MikroC for PIC version 6.2*.

Recommended PC requirements for this system:

* Windows 7 or newer operating system.
* DDC/CI support monitor or display device.
* PC with USB 1.1 or newer port(s).