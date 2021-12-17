# E-Paper_Pricetags


### You can support my work via PayPal: https://paypal.me/hoverboard1 this keeps projects like this coming.


## 11.6" Version
- To use it with Arduino and GxEPD2 copy the modded library into your Arduino library folder and open the GxEPD2_Example and uncomment the GxEPD2_1160c display, its under ESP32 section, if you want to use it with a different Arduino you need to copy it or edit it for that one / pinout

## 4.2" Version
- This one is directly compatible with the GxEPD2 library.
- To modify the stock pcb take a look at my video manual: https://youtu.be/c62D3Z-c5IM

## 4.4" Version Imagotag G1 4.4 BWR

### Stock PCB external control
- Use the ESP32 Demo code in the folder Imagotag_G1_4.4_BWR
- To modify the stock pcb take a look at my video manual and the image in the folder: https://youtu.be/xxx

### Stock Firmware using custom AP
- Manual and code coming later video with demo here: https://youtu.be/DhEPJjdtTQc

## 6" Version
- This one should be compatible with the GxEPD2 library but is not tested, it is recomendet to Hack the stock PCB by adding a USB connector and a 5V bodge wire and reflash the MZ100 SOC to enable USB

## 7.5" Version

### Arduino
- To use it with Arduino and GxEPD2 copy the modded library into your Arduino library folder and open the GxEPD2_Example and uncomment the GxEPD2_750c_F19 display, its under ESP32 section, if you want to use it with a different Arduino you need to copy it or edit it for that one / pinout

- Here is a good manual on soldering the needed wires https://hackaday.io/project/175947-74-e-ink-shelf-label-used-as-a-weather-station

### Stock PCB / CC1110
- To use the Stock PCB you can use the SDCC Example and compile your own code via the Combile.bat file in windows, currently it simply refreshes the display but its working.
can be flashed via the CC-Debugger or another compatible programmer.

- *Update* full custom firmware, http://dmitry.gr/?r=05.Projects&proj=29.%20eInk%20Price%20Tags

### Stock firmware using custom AP
- To use the display with stock firmware and stock pcb you can use the ESP32 AccessPoint Emulator from the folder Custom_PriceTag_AccesPoint
You need these two librarys: 
https://github.com/me-no-dev/ESPAsyncWebServer
And https://github.com/espressif/arduino-esp32
Also a CC1101 868mhz! Module is needed, make shure to get a real 868mhz version, some seller write 868 when its an 433 module, the ebyte E07-868 module is proven to give good results

- A Youtube livestream with a long walktrough and overal explanation is available here:
https://youtu.be/pRoFim3Egss
