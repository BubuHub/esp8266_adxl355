# ESP8266 ADXL355 sensor example

This simple program reads values from the FIFO of the ADXL355 chip over the SPI bus and displays the results on the UART port.
The hardware SPI bus support was placed in hspi.[ch] file, I wrote my own SPI driver because standard SPI driver from Arduino caused interrupts between bytes. 
The SPI of the ESP8266 chip is very easy and fun to program, so I decided to write my own driver.

![](https://github.com/BubuHub/esp8266_adxl355/blob/main/blob/assets/spi_my.png)

The program written in QT5 (qt-logger) located in pc/qt-logger directory allows me to save data to csv file.

The test project was built on D1 mini board connected to PC computer over USB as /dev/ttyUSB0 under Linux.
Modify the serial port device name in platformo.ini if you want to compile under Windows.

The project uses platformio build environment. 
[PlatformIO](https://platformio.org/) - Professional collaborative platform for embedded development.

# Hardware connections

| D1_MINI           |   ADXL255  |
| :---------------: | :--------: |
| GND               | GND        |
| 3.3V              | VCC        |
| D5 GPIO14 (SCL)   | SCLK       |
| D6 GPIO12 (MISO)  | MISO       |
| D7 GPIO13 (MOSI)  | MOSI       |
| D8 GPIO15 (CS)    | CS         |

# Big fat warning
Do not use USB3.x ports under Windows. Use USB2.0 if possible. 
I don't know why, but connecting the D1 mini to USB3.x in Windows even through a USB2.0 HUB causes some data loss at 1.5Mb transfer rate.  Maybe the CH340 driver under Windows is to blame because Linux has no problems with it.


# Building under Linux
* install PlatformIO
* enter project directory
* connect D1 mini board to PC computer over USB cable.
* type in terminal:
  platformio run -t upload  

You can also use IDE to build this project on Linux/Windows/Mac. My fvorite ones:
* [Code](https://code.visualstudio.com/) 
* [Atom](https://atom.io/)

Enjoy :-)
