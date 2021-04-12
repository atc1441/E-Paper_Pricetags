#include <Arduino.h>
#include <SPI.h>
#include "RFV3.h"
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "class.h"
#include "interval_timer.h"
#include <FS.h>
#if defined(ESP32)
#include "SPIFFS.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include "utils.h"

byte nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;
}

void hexCharacterStringToBytes(byte *byteArray, String hexString)
{
  bool oddLength = hexString.length() & 1;

  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < hexString.length(); charIndex++)
  {
    bool oddCharIndex = charIndex & 1;
    if (oddLength)
    {
      if (oddCharIndex)
      {
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
    else
    {
      if (!oddCharIndex)
      {
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

void tohex(unsigned char *in, size_t insz, char *out, size_t outsz)
{
  unsigned char *pin = in;
  const char *hex = "0123456789ABCDEF";
  char *pout = out;
  for (; pin < in + insz; pout += 3, pin++)
  {
    pout[0] = hex[(*pin >> 4) & 0xF];
    pout[1] = hex[*pin & 0xF];
    pout[2] = ':';
    if (pout + 3 - out > outsz)
    {
      break;
    }
  }
  pout[-1] = 0;
}

void print_buffer(uint8_t *buffer, int len){
    printf("Len %d Data:", len);
    for (int i = 0; i < len; i++)
    {
      printf(" 0x%02x", buffer[i]);
    }
    printf("\r\n");
}
