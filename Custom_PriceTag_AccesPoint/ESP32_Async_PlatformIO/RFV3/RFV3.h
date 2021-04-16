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
#include "web.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include "utils.h"
#include "trans_assist.h"
#include "wu_assist.h"

#define CLK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19
#define SS_PIN 5

//#define GDO0 32 not used currently
#define GDO2 4

void log(String message);

void set_mode_idle();
void set_mode_sync();
void set_mode_full_sync();
void set_mode_trans();
void set_mode_wu();
void set_mode_wu_reset();
void set_mode_wu_activation();
void set_mode_wun_activation();
void set_mode_activation();
String get_mode_string();
