
#include "G1_Driver.h"
#define EPD_CLK    18
#define EPD_MOSI   23
#define EPD_CS     5
#define EPD_ENABLE 17
#define EPD_BUSY   16

#define EPD_WIDTH 400
#define EPD_HEIGHT 300

G1 epd(EPD_WIDTH, EPD_HEIGHT);
GFXcanvas1 red(EPD_WIDTH, EPD_HEIGHT);// Create a canvas for the RED pixels

void setup() {
  Serial.begin(115200);
  delay(1000);// wait till Serial Terminal on PC is ready

  epd.begin(EPD_MOSI, EPD_CLK, EPD_CS, EPD_BUSY, EPD_ENABLE);

}

void loop() {
  epd.fillScreen(0x00);//Print something in the Black buffer

  epd.setCursor(0, 0);
  epd.setTextSize(3);
  epd.setTextColor(1, 0);
  epd.printf("ATCnetz.de\r\n");
  epd.printf("E-Paper 400x300\r\n");
  epd.printf("Black ");
  epd.setTextColor(0, 1);
  epd.printf(" White \r\n\r\n");
  epd.setTextColor(1, 0);
  epd.printf("Arduino ESP32\r\n");

  epd.setCursor(0, 160);
  epd.setTextSize(3);
  epd.printf("Hello %i\r\n", millis());

  red.fillScreen(0x00);//Print something in the Red buffer

  red.setCursor(0, 0);
  red.setTextSize(3);
  red.printf("\r\n\r\n              Red\r\n");

  red.fillCircle(300, 200, 55, 1);

  red.setCursor(0, 200);
  red.setTextSize(3);
  red.printf("RED %i\r\n", millis());

  epd.temperature(0x50);// Set the enviromental temperature as the Display needs to be driven longer if its colder, lower number = colder
  Serial.println(epd.display(red.getBuffer()));
  delay(10000);
}
