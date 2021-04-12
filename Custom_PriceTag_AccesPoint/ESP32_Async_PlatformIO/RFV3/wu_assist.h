#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "utils.h"
#include "interval_timer.h"
#include <FS.h>
#if defined(ESP32)
#include "SPIFFS.h"
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <Hash.h>
#endif
#include "arith.h"
#include "settings.h"

#define WU_ACTIVATION_CMD 0
#define WU_WAKEUP_CMD 1
#define WU_RESET_CMD 2
#define WU_NEW_ACTIVATION_CMD 3

#define NEW_ACTIVATION_SLOTS 4
#define NEW_ACTIVATION_FREQ 8
#define NEW_ACTIVATION_NETID 254

void start_wu(uint8_t cmd);
bool handle_wu();
void check_wu_timeout();
