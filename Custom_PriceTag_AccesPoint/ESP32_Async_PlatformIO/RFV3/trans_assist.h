#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "logger.h"
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "interval_timer.h"
#include <FS.h>
#include <SPIFFS.h>

int get_len_send();
void reset_trans_values();

void get_trans_part(int position, int length, uint8_t *buffer);
void set_trans_buffer(uint8_t *buffer, int length);
int set_trans_file(String &path);
void end_trans_file();
bool get_trans_file_open();

int get_still_to_send();
void set_still_to_send(int state);

void add_to_receive_buffer(uint8_t *buffer);
void save_receive_buffer();
String get_last_receive_string();