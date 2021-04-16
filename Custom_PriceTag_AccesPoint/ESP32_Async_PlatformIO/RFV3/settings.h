#pragma once
#include <FS.h>
#if defined(ESP32)
#include "SPIFFS.h"
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <Hash.h>
#endif

void read_boot_settings();
void save_settings_to_flash();
void delete_settings_file();

void appendFile(const char *path, String message);
void deleteFile(const char *path);


String split(String s, char parser, int index);
