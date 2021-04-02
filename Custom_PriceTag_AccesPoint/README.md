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



## TODO list
- Handling the ACK received from the display better, now it only compares the exprected one to the received one and resends all blocks but only needs to send the missed parts again. also parts with len of 1-7 go into a resend loop as not handled correctly if not received full on first send.
- Get a better Recover method, now its very manual to get a missconfigured displays back in business
- create a database with all activated displays with its type and IDs to make automatic sending of dynamic data simpler
- Implement Larrys dynamic oneBitLibrary fully into the system
