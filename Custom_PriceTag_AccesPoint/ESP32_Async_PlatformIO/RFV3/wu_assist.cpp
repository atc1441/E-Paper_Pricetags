#include "wu_assist.h"
#include <Arduino.h>
#include <SPI.h>
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "utils.h"
#include "interval_timer.h"
#include <FS.h>
#include <SPIFFS.h>
#include "arith.h"
#include "settings.h"

int wu_period_ms = 15500;
int wu_period_ms_timeout = wu_period_ms + 1000;

long wu_start_time = 0;

uint8_t wu_buffer[10];

void start_wu(uint8_t cmd)
{
    wu_start_time = millis();

    wu_buffer[0] = 0x00;
    if (cmd == WU_NEW_ACTIVATION_CMD)
    {
        wu_buffer[1] = 0xff;
        wu_buffer[2] = 0x05;                    // Num Periods per slot
        wu_buffer[3] = 0x4c;                    // Slot time in MS LOW
        wu_buffer[4] = 0x04;                    // Slot time in MS HIGH
        wu_buffer[5] = NEW_ACTIVATION_SLOTS;    // Num Slots Activation
        wu_buffer[6] = 0x02;                    // maybe max missed syncs
        wu_buffer[7] = NEW_ACTIVATION_FREQ + 1; // Frequenzy
        wu_buffer[8] = 0x03;                    // CMD mode = New Activation
        wu_buffer[9] = NEW_ACTIVATION_NETID;    // Used NetID for Activation
    }
    else
    {
        if (cmd == WU_ACTIVATION_CMD)
        {
            uint8_t serial[6];
            get_serial(serial);
            memcpy(&wu_buffer[1], serial, 6);
        }
        else
        {
            memset(&wu_buffer[1], 0xff, 6);
        }
        wu_buffer[7] = get_freq() + 1;
        wu_buffer[8] = cmd;
        wu_buffer[9] = get_network_id();
    }
    print_buffer(wu_buffer, sizeof(wu_buffer));

    cc1101_prepaire_tx(get_wu_channel(), 0);
    cc1101_tx_fill(wu_buffer, sizeof(wu_buffer));
    cc1101_tx();
}

bool handle_wu()
{
    if (millis() - wu_start_time > wu_period_ms)
        return true;
    cc1101_tx_fill(wu_buffer, sizeof(wu_buffer));
    return false;
}

void check_wu_timeout()
{
    if (millis() - wu_start_time > wu_period_ms_timeout)
    {
        log("Something wrong, back to idle");
        set_mode_idle();
    }
}