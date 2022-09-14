#include "Adafruit_GFX1.h"
#include "EinkDisplay.h"

#define xy_swap(a, b) \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

EinkDisplay::EinkDisplay(SPIClass& spi, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, int8_t busy_pin) :
  Adafruit_GFX(EinkDisplay_HEIGHT, EinkDisplay_WIDTH),
  spi(spi), dcPin(dc_pin), csPin(cs_pin), busyPin(busy_pin), rstPin(rst_pin) {
  spiSettings = SPISettings(4000000, MSBFIRST, SPI_MODE0);
}

boolean EinkDisplay::begin() {
  clearDisplay();
  pinMode(busyPin, INPUT);
  digitalWrite(csPin, HIGH);
  pinMode(csPin, OUTPUT);
  digitalWrite(dcPin, HIGH);
  pinMode(dcPin, OUTPUT);
  digitalWrite(rstPin, HIGH);
  pinMode(rstPin, OUTPUT);
  spi.begin();
  return true;
}


void EinkDisplay::display(void) {
  uint16_t eink_x = ((EinkDisplay_WIDTH - 1) / 8);
  uint16_t eink_y = (EinkDisplay_HEIGHT - 1);

  delay(20);
  digitalWrite(rstPin, LOW);
  delay(20);
  digitalWrite(rstPin, HIGH);
  delay(130);

  //Looks Like https://download.mikroe.com/documents/datasheets/e-paper-display-2%2C9-296x128-n.pdf
  spi.beginTransaction(spiSettings);

  _writeCommand(0x12);//Software reset
  _waitWhileBusy();
  delay(5);

  _writeCommand(0x74);//Set Analog Block Control
  _writeData (0x54);

  _writeCommand(0x7E);//Set Digital Block Control
  _writeData (0x3B);

  _writeCommand(0x2B);//ACVCOM setting
  _writeData (0x04);
  _writeData (0x63);

  _writeCommand(0x0C); // Softstart Control
  _writeData (0x8F);
  _writeData (0x8F);
  _writeData (0x8F);
  _writeData (0x3F);

  _writeCommand(0x01);//Driver Output control
  _writeData (eink_y);
  _writeData ((eink_y >> 8));
  _writeData (0x00);

  _writeCommand(0x11);//Data Entry mode setting
  _writeData (0x01);

  _writeCommand(0x44);//Set RAM X - address Start/End position
  _writeData (0x00);
  _writeData (eink_x);

  _writeCommand(0x45);//Set RAM Y - address Start/End position
  _writeData (eink_y);
  _writeData ((eink_y >> 8));
  _writeData (0x00);
  _writeData (0x00);

  _writeCommand (0x3C);//Border Waveform Control
  _writeData (0x01);//0 = black,1 = white,2 = Red

  _writeCommand (0x18);// Temperature sensor control
  _writeData (0x80);// 0x48 = External,0x80 = Internal

  _writeCommand (0x21);//Display Update Control 1
  _writeData (B00001000);//inverse or ignore ram content

  _writeCommand (0x22);//Display Update Control 2
  _writeData (0xB1);

  _writeCommand (0x20);//Master Activation
  _waitWhileBusy();
  delay(5);

  _writeCommand (0x4E);//Set RAM X address counter
  _writeData (0x00);

  _writeCommand (0x4F);//Set RAM Y address counter
  _writeData (eink_y);
  _writeData ((eink_y >> 8));

  _writeCommand (0x26);//Write RAM RED

  for (uint32_t i = 0; i < EinkDisplay_BUFFER_SIZE; i++)
  {
    _writeData(_red_buffer[i]);
  }

  _writeCommand (0x4E);//Set RAM X address counter
  _writeData (0x00);

  _writeCommand (0x4F);//Set RAM Y address counter
  _writeData (eink_y);
  _writeData ((eink_y >> 8));

  _writeCommand (0x24);//WRITE RAM BW

  for (uint32_t i = 0; i < EinkDisplay_BUFFER_SIZE; i++)
  {
    _writeData(_black_buffer[i]);
  }

  _writeCommand (0x22);//Display Update Control 2
  _writeData (0xC7);

  _writeCommand (0x20);//Master Activation
  _waitWhileBusy();
  _writeCommand (0x10);//Deep Sleep mode
  _writeData (B00000001);
  spi.endTransaction();
}

void EinkDisplay::_writeCommand(uint8_t command)
{
  digitalWrite(dcPin, LOW);
  digitalWrite(csPin, LOW);
  spi.transfer(command);
  digitalWrite(csPin, HIGH);
}

void EinkDisplay::_writeData(uint8_t data)
{
  digitalWrite(dcPin, HIGH);
  digitalWrite(csPin, LOW);
  spi.transfer(data);
  digitalWrite(csPin, HIGH);
}


void EinkDisplay::_waitWhileBusy()
{
  while (1)
  {
    if (digitalRead(busyPin) == 0) break;
  }
}

void EinkDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    xy_swap(x, y);
    switch (getRotation()) {
      case 1:
        xy_swap(x, y);
        x = WIDTH - x - 1;
        break;
      case 2:
        x = WIDTH  - x - 1;
        y = HEIGHT - y - 1;
        break;
      case 3:
        xy_swap(x, y);
        y = HEIGHT - y - 1;
        break;
    }
    uint16_t i = x / 8 + y * EinkDisplay_WIDTH / 8;

    _red_buffer[i] = (_red_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    _black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    if (color == RED)_red_buffer[i] = (_red_buffer[i] | (1 << (7 - x % 8)));
    else if (color == BLACK) _black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8)));
  }
}

void EinkDisplay::clearDisplay(void) {
  for (uint16_t x = 0; x < EinkDisplay_BUFFER_SIZE; x++)
  {
    _black_buffer[x] = 0x00;
    _red_buffer[x] = 0x00;
  }
}

void EinkDisplay::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  for (uint16_t i = 0; i < w; i++)
  {
    drawPixel(x + i, y, color);
  }
}

void EinkDisplay::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  for (uint16_t i = 0; i < h; i++)
  {
    drawPixel(x, y + i, color);
  }
}
