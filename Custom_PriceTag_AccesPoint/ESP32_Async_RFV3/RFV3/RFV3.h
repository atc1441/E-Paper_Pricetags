#pragma once
#include "class.h"

#define CLK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19
#define SS_PIN 5

#define GDO2 4

void set_mode_idle();
void set_mode_sync();
void set_mode_full_sync();
void set_mode_trans();
void set_mode_wu();
void set_mode_wu_activation();
void set_mode_wun_activation();
void set_mode_activation();
void set_mode_sniff();
String get_mode_string();
