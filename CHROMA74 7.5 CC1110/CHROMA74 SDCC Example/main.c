  
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cc1110.h>
#include <mcs51/8051.h>
#include "ioCCxx10_bitdef.h"
#include "bits.h"

#define epd_enable P0_6
#define led P1_4

#define epd_bs P0_0
#define epd_dc P0_7
#define epd_busy P1_0
#define epd_cs P1_1
#define epd_reset P1_2
#define epd_clk P1_3
#define epd_mosi P1_5

uint8_t lut20[] = {
  0x20, 0x01, 0x00, 0x00, 0x64, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x03, 0x01, 0x03, 0x05, 0x07, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x50, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x50, 0x28, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x50, 0x28, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
uint8_t lut22[] = {
  0x22, 0x01, 0x00, 0x00, 0x00, 0x00, 0x64, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x10, 0x00, 0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x11, 0x12, 0x00, 0x00, 0x03, 0x01, 0x03, 0x05, 0x07, 0x00, 0x00, 0x00, 0x0A, 0x12, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x12, 0x00, 0x00, 0x00, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x20, 0x00, 0x00, 0x00, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x50, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
uint8_t lut21[] = {
  0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x64, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x10, 0x00, 0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x11, 0x02, 0x20, 0x00, 0x03, 0x01, 0x03, 0x05, 0x07, 0x00, 0x00, 0x00, 0x0A, 0x12, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x12, 0x00, 0x00, 0x00, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x01, 0x10, 0x00, 0x00, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x50, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
uint8_t lut25[] = {
  0x25, 0x01, 0x00, 0x00, 0x00, 0x00, 0x64, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x11, 0x02, 0x20, 0x00, 0x03, 0x01, 0x03, 0x05, 0x07, 0x00, 0x00, 0x00, 0x0A, 0x12, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x12, 0x00, 0x00, 0x00, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x03, 0x30, 0x00, 0x00, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x30, 0x20, 0x00, 0x00, 0x50, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x30, 0x20, 0x00, 0x00, 0x50, 0x28, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x30, 0x20, 0x00, 0x00, 0x50, 0x28, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
uint8_t lut29[] = {
  0x29, 0x01, 0x3F, 0x64, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xFF, 0x03, 0x01, 0x03, 0x05, 0x07, 0x00, 0x00, 0x00, 0x0A, 0xFF, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xFF, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xFF, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xFF, 0x50, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0x50, 0x28, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x50, 0x28, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void xtalClock() { // Set system clock source to 26 Mhz
  SLEEP &= ~SLEEP_OSC_PD; // Turn both high speed oscillators on
  while( !(SLEEP & SLEEP_XOSC_S) ); // Wait until xtal oscillator is stable
  CLKCON = (CLKCON & ~(CLKCON_CLKSPD | CLKCON_OSC)) | CLKSPD_DIV_1; // Select xtal osc, 26 MHz
  while (CLKCON & CLKCON_OSC); // Wait for change to take effect
  SLEEP |= SLEEP_OSC_PD; // Turn off the other high speed oscillator (the RC osc)
}

void delay(int ms) {
  int j;
  while (--ms > 0) {
  	for (j=0; j<1650;j++); // about 1 millisecond
  };
}

void wait_busy() {
  delay(2);
  while(epd_busy == 0){}
}

void configureSPI() {
  PERCFG |= 1;// SPI Alternative pins
  U0CSR = 0x40;  //Set SPI Master operation
  U0BAUD =  0; // set Mantissa
  U0GCR = 0x31; // set clock on 1st edge, -ve clock polarity, MSB first, and exponent
}

void tx(unsigned char ch) {
  epd_cs = 0;
  U0DBUF = ch;
  while(!(U0CSR & U0CSR_TX_BYTE)); // wait for byte to be transmitted
  U0CSR &= ~U0CSR_TX_BYTE;         // Clear transmit byte status
  epd_cs = 1;
}

void epd_cmd(uint8_t Reg)
{
  epd_dc = 0;
  tx(Reg);
}

void epd_data(uint8_t Data)
{
  epd_dc = 1;
  tx(Data);
}

void send_lut(uint8_t lut[]) {
  epd_cmd(lut[0]);
  for (int r = 1; r <= sizeof(lut20); r++)
  {
    epd_data(lut[r]);
  }
}

void send_empty_lut(uint8_t lut) {
  epd_cmd(lut);
  for (int r = 0; r <= 260; r++)
    epd_data(0x00);
}


void EPD_7IN5_Init(void)
{

  epd_enable = 1;
  epd_bs = 1;
  epd_dc = 1;
  epd_cs = 1;
  epd_reset = 1;
  delay(100);
  
  epd_enable = 0;
  epd_bs = 0;
  epd_cs = 1;
  
  epd_reset = 0;
  delay(100);
  epd_reset = 1;
  delay(100);
	
  epd_cmd(0x61);
  epd_data(0x02);
  epd_data(0x80);
  epd_data(0x01);
  epd_data(0x80);

  epd_cmd(0x06);
  epd_data(0x0e);
  epd_data(0xcd);
  epd_data(0x26);

  epd_cmd(0x00);
  epd_data(0xEF);
  epd_data(0x80);

  epd_cmd(0x03);
  epd_data(0x00);

  epd_cmd(0x82);
  epd_data(0x22);

  epd_cmd(0x60);
  epd_data(0x22);

  epd_cmd(0x41);
  epd_data(0x80);

  epd_cmd(0x50);
  epd_data(0x77);

  epd_cmd(0x65);
  epd_data(0x00);

  //Send data to be displayed
  epd_cmd(0x10);
   for (long r = 1; r <= 122880; r++)
    {
      //0000 = Black
      //0100 = Yellow
      //0011 = White
     epd_data(r);
    }
	
  //**Here is LUT special stuff **//
  epd_cmd(0x01);
  epd_data(0x07);
  epd_data(0x01);
  epd_data(0x01);
  epd_data(0x3c);

  epd_cmd(0x30);
  epd_data(0x3A);

  //Unused LUTs need to be 0x00
  send_empty_lut(0x23);
  send_empty_lut(0x24);
  send_empty_lut(0x26);
  send_empty_lut(0x27);
  send_empty_lut(0x28);

  //Send LUTs from array
  send_lut(lut20);
  send_lut(lut22);
  send_lut(lut21);
  send_lut(lut25);
  send_lut(lut29);
  //**LUT special stuff end**//

  //Power ON
  epd_cmd(0x04);
  wait_busy();

  epd_cmd(0x12);
  wait_busy();

  //Power OFF
  epd_cmd(0x02);

  //Sleep
  epd_cmd(0x07);
  epd_data(0xA5);
}

void main() {
  xtalClock();
  
  P1SEL |= (BIT5 | BIT3 ); // set SCK and MOSI as peripheral outputs
  P1DIR |= (BIT1 | BIT2 | BIT4);
  P0DIR |= (BIT0 | BIT6 | BIT7);  
  
  P1DIR &= ~(BIT0);
  
  configureSPI();
  
  EPD_7IN5_Init();
  
  while (1) {
  	led = !led;
  	delay(100);
  }
}
