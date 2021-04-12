#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "interval_timer.h"
#include <FS.h>
#if defined(ESP32)
#include "SPIFFS.h"
#include <ESPmDNS.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <Hash.h>
#endif

typedef struct _bmp_s_tag
{
    int height;
    int width;
    int offset;
    int pitch;
    int bsize;
    int bTopDown;
    int header_size;
    uint16_t checksum;
} _bmp_s;

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

int get_compress_method_from_size(int width, int height, int *big_address);
uint8_t open_bmp(String &path, _bmp_s *bmp_infos);
int load_img_to_bufer(String &path, String &path1, bool save_file_to_spiffs);
int load_img_to_bufer_none(File file_in, _bmp_s *bmp_infos);
int load_img_to_bufer_rle(File file_in, _bmp_s *bmp_infos);

int fill_header(uint8_t *buffer_out, int compression_size, int height, int width, int compression_type, int color, int header_size, uint16_t checksum);
