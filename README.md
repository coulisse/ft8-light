FT8 Light
===

**THIS CODE IS NOT WORKING**: WORK IN PROGRESS! STAY TUNED!

### Ham radio FT8 and FT4 portable companion
[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](http://perso.crans.org/besson/LICENSE.html)
[![made-with-c++](https://img.shields.io/badge/C++-Solutions-blue.svg?style=flat&logo=c++)](https://img.shields.io/badge/C++-Solutions-blue.svg?style=flat&logo=c++)
[![stability-alpha](https://img.shields.io/badge/stability-alpha-f4d03f.svg)](https://github.com/mkenney/software-guides/blob/master/STABILITY-BADGES.md#alpha)


- **Release:** v0.1
- **Author:** Corrado Gerbaldo - IU1BOW.
- **Mail:** <corrado.gerbaldo@gmail.com>
- **Licensing:** Gpl V3.0 see ["LICENSE"](LICENSE) file.
- **Languages:** This application is written in C/C++
- **My website:** [https://www.iu1bow.it](https://www.i1bow.it) 
___
**FT8 Light**   
The purpose of this project is to make a  portable "modem" for  **FT8/FT4** ham-radio **digital mode**, usable in  QRP mode, with Yaesu FT897, FT857 and FT817 radio.
It could be also adaptable for others radio.

**Library dependancies**
- [u8g2](https://github.com/olikraus/u8g2)
- SPI
- FreeRTOS
- [AudioKitHAL](https://github.com/pschatzmann/arduino-audiokit)

**Hardware** 
- ESP32-A1S Audio Kit
- oled display 128x64 I2C

**References**
- [The ‘AI Thinker Audio Kit’ Arduino Challenge (Published by pschatzmann)](https://www.pschatzmann.ch/home/2021/12/06/the-ai-thinker-audio-kit-experience-or-nothing-is-right/)
- I used the [kgoba/ft8_lib](https://github.com/kgoba/ft8_lib) as base, sliglty adapting them for this project.

### Changelog
see file ["CHANGELOG.md"](docs/CHANGELOG.md)

### Board configuration
Set the ESP32-A1S Audio Kit swithes in the following mode: 
- 1=off
- 2=on
- 3=on
- 4=off
- 5=off

### Setup
*TODO*

### Compiling
If you are using Arduino IDE, compile with log level *info*, from menu: tools --> core debut level --> *info*

### Schema            
*TODO*
