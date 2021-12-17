#include <SPI.h>
#include "G1_Driver.h"
#include <Arduino.h>

G1::G1(uint16_t w, uint16_t h): GFXcanvas1(w, h) {}

void G1::begin(int8_t epd_mosi, int8_t epd_clk, int8_t epd_cs, int8_t epd_busy, int8_t epd_enable) {
  _epd_mosi = epd_mosi;
  _epd_clk = epd_clk;
  _epd_cs = epd_cs;
  _epd_busy = epd_busy;
  _epd_enable = epd_enable;

  vspi = new SPIClass(VSPI);
  vspi->begin(_epd_clk, -1, _epd_mosi, -1); //SCLK, MISO, MOSI, SS
  pinMode(_epd_cs, OUTPUT);
  pinMode(_epd_enable, OUTPUT);
  pinMode(_epd_busy, INPUT);
  digitalWrite(_epd_enable, LOW);
  digitalWrite(_epd_cs, LOW);
}

uint8_t G1::display(uint8_t*red_buffer) {
  int retry = 10;
  while (display_internal(red_buffer)) {
    digitalWrite(_epd_enable, LOW);
    digitalWrite(_epd_cs, LOW);
    delay(500);
    if (retry-- <= 0) {
      return 1;
    }
  }
  return 0;
}

uint8_t G1::display_internal(uint8_t*red_buffer) {
  if (epd_begin(_temperature)) {
    return 3;
  }

  int i, x;
  int epd_timeout = 1000;//ms
  long epd_start_time;
  uint32_t buffer_position = 0;
  for (x = 0; x < 300; x++) {
    for (i = 0; i < 50; i++) {
      if (red_buffer != 0)
        epd_send_spi(red_buffer[buffer_position++]);//color RED
      else
        epd_send_spi(0x00);//color RED
    }
    delay(1);
    epd_start_time = millis();
    while (digitalRead(_epd_busy) == HIGH) {
      if (millis() - epd_start_time > epd_timeout)
        return 2;
    }
  }
  buffer_position = 0;
  for (x = 0; x < 300; x++) {
    for (i = 0; i < 50; i++) {
      epd_send_spi(this->getBuffer()[buffer_position++]);//color BLACK
    }
    delay(1);
    if (x == 299)
      break;
    epd_start_time = millis();
    while (digitalRead(_epd_busy) == HIGH) {
      if (millis() - epd_start_time > epd_timeout)
        return 1;
    }
  }
  delay(50);

  digitalWrite(_epd_cs, HIGH);
  while (digitalRead(_epd_busy) == HIGH) {}
  digitalWrite(_epd_enable, LOW);
  digitalWrite(_epd_cs, LOW);
  return 0;
}

void G1::epd_send_spi(uint8_t data) {
  vspi->beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));
  vspi->transfer(data);
  vspi->endTransaction();
  delayMicroseconds(10);
}

uint8_t G1::epd_begin(uint8_t temperature) {
  digitalWrite(_epd_enable, HIGH);
  delay(300);
  digitalWrite(_epd_cs, HIGH);
  delay(300);
  digitalWrite(_epd_cs, LOW);
  delay(1);
  epd_send_spi(0x88);
  epd_send_spi(temperature);
  delay(1);

  if (digitalRead(_epd_busy) == LOW)
    return 2;//BUSY Did not go high so something is wrong

  int epd_timeout = 1000;//ms
  long epd_start_time = millis();
  while (digitalRead(_epd_busy) == HIGH) {
    if (millis() - epd_start_time > epd_timeout) {
      return 1;
    }
  }
  return 0;
}

void G1::temperature(uint8_t temp){
  _temperature = temp;
}
