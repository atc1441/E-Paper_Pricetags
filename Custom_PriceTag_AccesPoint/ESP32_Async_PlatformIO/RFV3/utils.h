#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "RFV3.h"
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "class.h"
#include "interval_timer.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

byte nibble(char c);
void hexCharacterStringToBytes(byte *byteArray, String hexString);
void tohex(unsigned char * in, size_t insz, char * out, size_t outsz);

void print_buffer(uint8_t *buffer, int len);