Standard Peripheral Library for microcontrollers.
The goal is to develop a common interface usable across different microcontrollers.
Currently working on NXP (LPC1768), hopefully would be able to extend to other uC (if I have the money to buy the development kits, and the time).

Implemented Features:
GPIO - Normal I/O, interrupt
UART
I2C
SPI
ADC, burst mode(hardware) and software mode.
DAC, sample application is a simple piano, where DO maps to A, RE maps to S, etc. Use the ADC as a volume (not so nice, but can demonstrate the use of ADC and DAC.

Hoping to implement the following features next:
SSPI
Timers

And maybe USB and Ethernet.

The barebone test file is at app/main_barebone.c.
To compile with freeRTOS (not much in here, just a pair of LEDs blinking alternately at 100ms interval), edit the makefile and change 
BAREBONE	   = 0

The library is at lib & utils. Other directories/files were copied  (modified here and there) from either freeRTOS.org or http://www.emb4fun.de/archive/eclipse/ or http://www.emb4fun.de/arm/examples/index.html .

Compiled using eclipse + gcc. There is a very good tutorial here how to setup gcc on eclipse http://www.emb4fun.de/archive/eclipse/.