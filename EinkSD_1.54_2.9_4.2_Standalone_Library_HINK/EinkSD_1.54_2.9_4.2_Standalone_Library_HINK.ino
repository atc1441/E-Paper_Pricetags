
#include <SPI.h>
#include "Adafruit_GFX1.h"
#include "EinkDisplay.h"

// CLK 18
// MOSI 23
#define OLED_DC    17
#define OLED_CS    5
#define OLED_RESET 16
#define OLED_BUSY 4
EinkDisplay display(SPI, OLED_DC, OLED_RESET, OLED_CS, OLED_BUSY);


void setup() {
  Serial.begin(115200);
  display.begin();
  display.setRotation(0);
  display.setTextSize(2);
  display.setTextColor(RED);
}

void loop() {
  display.clearDisplay();
  display.fillCircle(30, 30, 14, BLACK);
  display.fillCircle(80, 30, 14, RED);
  display.setCursor(0, 0);
  display.println("Was geht so");
  display.println(millis());


  display.println();
  display.display();

  delay(20000);
}
