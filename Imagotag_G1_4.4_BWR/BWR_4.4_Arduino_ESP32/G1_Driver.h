#pragma once
#include <SPI.h>
#include <Adafruit_GFX.h>

class G1 : public GFXcanvas1 {
  public:
    G1(uint16_t w, uint16_t h);
    void begin(int8_t epd_mosi, int8_t epd_clk, int8_t epd_cs, int8_t epd_busy, int8_t epd_enable);
    uint8_t display(uint8_t*red_buffer = 0);
    void temperature(uint8_t temp);

  private:
    uint8_t display_internal(uint8_t*red_buffer = 0);
    void epd_send_spi(uint8_t data);
    uint8_t epd_begin(uint8_t temperature);

    SPIClass * vspi = NULL;
    int8_t _epd_mosi = -1;
    int8_t _epd_clk = -1;
    int8_t _epd_cs = -1;
    int8_t _epd_busy = -1;
    int8_t _epd_enable = -1;
    uint8_t _temperature = 0x50;
};
