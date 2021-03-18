#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "RFV3.h"
#include "main_variables.h"
#include "logger.h"
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

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void init_web();
void appendFile(const char * path, String message);
void deleteFile(const char * path);
