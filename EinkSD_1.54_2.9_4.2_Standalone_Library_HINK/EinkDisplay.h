#ifndef _EinkDisplay_H_
#define _EinkDisplay_H_
/*1,54inch*/
#define EinkDisplay_WIDTH 152
#define EinkDisplay_HEIGHT 152  //from controller and up

/*2,9inch
#define EinkDisplay_WIDTH 128
#define EinkDisplay_HEIGHT 296  //from controller and up
*/
/*4.2inch
#define EinkDisplay_WIDTH 400
#define EinkDisplay_HEIGHT 300  //from controller and up
*/
#define EinkDisplay_BUFFER_SIZE (uint32_t(EinkDisplay_WIDTH) * uint32_t(EinkDisplay_HEIGHT) / 8)

#include <SPI.h>
#include "Adafruit_GFX1.h"

#define WHITE                   0
#define BLACK                   1
#define RED                 2

class EinkDisplay : public Adafruit_GFX {
  public:
    EinkDisplay(SPIClass& spi, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, int8_t busy_pin);
    boolean      begin();
    void         display(void);
    void         clearDisplay(void);
    void         drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  private:
    void _writeCommand(uint8_t command);
    void _writeData(uint8_t data);
    void _waitWhileBusy();
    SPIClass& spi;
    uint8_t _black_buffer[EinkDisplay_BUFFER_SIZE];
    uint8_t _red_buffer[EinkDisplay_BUFFER_SIZE];
    int8_t       dcPin    ,  csPin,  busyPin, rstPin;
  protected:
    SPISettings  spiSettings;
};

#endif // _EinkDisplay_
