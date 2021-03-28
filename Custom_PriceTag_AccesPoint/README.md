## ESP32 Access Point for Electronic Pricetags.


This code works for the Following Pricetags:

Chroma74
Chroma29
EPOP50C
EPOP900

It should work on more displays from the Company ZBD / DisplayData but these are the tested ones.

To get it up and running flash the code with ArduinoIDE or PlatformIO to an ESP32 with a CC1101 868mhz module connect as in RFV3.h pinout
You need to rename the wlan_gitignore_replacement.h to wlan.h and supply your Wifi credentials. 
after the first boot you can navigate with a browser to *ip*/edit and upload the supplied index.htm then you can start controlling the AP from *ip*/edit


The supported pictures must be monochrom .bmp and one image for each color. 
the .bmp's need to have the same resolution as the wanted pricetag has


### If you are using a "green" CC1101 868mhz module and it fails make shure to set an offset of 15 in the settings as it turned out their osci is not the best.
