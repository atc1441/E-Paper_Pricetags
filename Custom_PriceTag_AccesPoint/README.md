# ESP32 Access Point for Electronic Pricetags.

A base station to wirelessly control a range of digital pricetags. Base station hardware consists of an ESP32 and CC1101 868 Mhz wireless module (most US-source modules are 433 MHz and are incompatable). This software currently works for the Following Pricetags:

* Chroma74
* Chroma29
* EPOP50C
* EPOP900/EPOP900RB

It should work on more displays from the Company ZBD / DisplayData but these are the ones which have been tested so far.

## Installation

This project is set up to use PlatformIO but it should also be possible to compile and flash using the Arduino IDE.
* Connect the CC1101 868 MHz radio module to the ESP32 using the pinout specified [in the RFV3.h header file](ESP32_Async_PlatformIO/RFV3/RFV3.h).
* Rename [wlan_gitignore_replacement.h](ESP32_Async_PlatformIO/RFV3/wlan_gitignore_replacement.h) to wlan.h and supply the desired credentials for the admin interface
* Build the project and upload to the ESP32
* To set up the WLAN credentials, connect to the access point called "AutoConnectAP" and navigate to http://192.168.4.1 (you should be redirected there automatically). Click on the name of your network (SSID), enter your WLAN password, and click on "Save*. The ESP32 will restart and will connect to the selected WLAN. In case the selected WLAN is not available, the ESP32 will automatically open the access point called "AutoConnectAP", allowing you to select another WLAN ([details](https://github.com/tzapu/WiFiManager))
* Navigate with a browser to *ip*/edit and upload the supplied [index.htm](ESP32_Async_PlatformIO/index.htm)
* Normal control of the AccessPoint can now be controlled by navigating to the IP address of the ESP32

## Radio Module Quirks

Testing shows the "green" CC1101 868 MHz modules have out-of-spec oscillators. The calibration registers can be used to compensate for this using the "Set base freq offset" of the web interface. Offsets of 15-17 have been shown to work well for these boards. If you cannot communicate with your displays, set this value and try again.

## Image Formats

The supported pictures must be monochrome .bmp, one image for each color in the case of multi-color diplays like the Chroma74. Images must have the same resolution and aspect ratio (landscape/portrait) for the target digital price tag or the display will not be updated even if the transmission is successful.

* Chroma74: 640x384
* EPOP900: 360x480

## Example Workflow

* Convert a sample image using the following ImageMagick command that forces 1-bit color, rotates, crops, and sizes for EPOP900 display:
  * `convert input.png -rotate 180 -resize "360x480^" -crop 360x480+0+0 -colorspace Gray -colors 2 -dither FloydSteinberg -type bilevel epop900demo.bmp`
* Upload the demo image to the ESP32 using the *ip*/edit page
* Activate new display using the web interface at the IP address of the ESP32:
  * Enter the display serial number, chose any ID number like, and click "Activate new display"
  * Successful activation will look something like: ```Activation: 0x0 0x47 0x41 0x0 0x22 0x15 0x84 0x83 0xFF 0x0 0x0 0x0 0x0 0x0 0x0 0x0
246998 Normal: RSSI: 176 LQI: 48
 Read_len:12 Data: 0xA 0x0 0x47 0x41 0x0 0x22 0x15 0x84 0x83 0xFF 0x0 0x43```
  * If you receive the message "no connection possible, exit activation" you may need to calibrate using the frequency offset
* Send the test image to the display:
  * Enter the ID number used during activation in the "Send command to display" ID box
  * Enter the filename in the "Filename:" box
  * Click "Send .bmp"
  * If you receive TX Timeout, check the ID or try adjusting the freq offset.

## TODO list
- Handling the ACK received from the display better, now it only compares the exprected one to the received one and resends all blocks but only needs to send the missed parts again.
- Get a better Recover method, now its very manual to get a missconfigured displays back in business
- create a database with all activated displays with its type and IDs to make automatic sending of dynamic data simpler
- Implement Larrys dynamic oneBitLibrary fully into the system
