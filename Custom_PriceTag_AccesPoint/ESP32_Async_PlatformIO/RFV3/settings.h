#pragma once
#include <FS.h>
#include <SPIFFS.h>

void read_boot_settings();
void write_setting(String setting, String state);

void appendFile(const char *path, String message);
void deleteFile(const char *path);


String split(String s, char parser, int index);