#pragma once
#include <FS.h>
#include <SPIFFS.h>

void read_boot_settings();
void save_settings_to_flash();
void delete_settings_file();

void appendFile(const char *path, String message);
void deleteFile(const char *path);


String split(String s, char parser, int index);