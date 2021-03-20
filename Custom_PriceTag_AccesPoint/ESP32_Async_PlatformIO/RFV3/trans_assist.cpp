#include "trans_assist.h"
#include <Arduino.h>
#include <SPI.h>
#include "logger.h"
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "utils.h"
#include "interval_timer.h"
#include <FS.h>
#include <SPIFFS.h>

#define _receive_size 0x1000
#define _send_size 0x4000

static File file;
int file_size = 0;
bool file_is_open = 0;

uint8_t data_to_send[_send_size] = {0};
uint8_t data_to_receive[_receive_size] = {0};

int length_to_send = 0;
int length_to_receive = 0;

String last_receive_string = "No data received so far";

int get_len_send()
{
    return length_to_send;
}

void reset_trans_values()
{
    length_to_send = 0;
    length_to_receive = 0;
    end_trans_file();
    memset(data_to_send, 0x00, _send_size);
    memset(data_to_receive, 0x00, _receive_size);
}

void get_trans_part(int position, int length, uint8_t *buffer)
{
    Serial.println("Getting trans part " + String(position));
    if (file_is_open)
    {
        file.seek(position, SeekSet);
        file.read(buffer, (size_t)length);
    }
    else
    {
        memcpy(buffer, &data_to_send[position], length);
    }
}

void set_trans_buffer(uint8_t *buffer, int length)
{
    reset_trans_values();
    length_to_send = length;
    memcpy(data_to_send, buffer, length);
}

int set_trans_file(String &path)
{
    reset_trans_values();
    file = SPIFFS.open(path, "rb");
    if (file == 0)
    {
        return 0;
    }
    file.seek(0, SeekEnd);
    file_size = file.position();
    file.seek(0, SeekSet);
    file_is_open = 1;
    length_to_send = file_size;
    return file_size;
}

void end_trans_file()
{
    if (file_is_open)
    {
        file_is_open = 0;
        file.close();
    }
}
bool get_trans_file_open()
{
    return file_is_open;
}

int still_to_send = 0;
int get_still_to_send()
{
    return still_to_send;
}

void set_still_to_send(int state)
{
    still_to_send = state;
}

void add_to_receive_buffer(uint8_t *buffer)
{
    memcpy(&data_to_receive[length_to_receive], buffer, 5);
    length_to_receive += 5;
}

void save_receive_buffer()
{
    char str[3 * length_to_receive];
    tohex(data_to_receive, length_to_receive, str, 3 * length_to_receive);
    String data_received = "NetID: ";
    data_received += String(get_network_id());
    data_received += " freq: ";
    data_received += String(get_freq());
    data_received += " display: ";
    data_received += String(get_display_id());
    data_received += " ";
    data_received += String(millis());
    data_received += " = ";
    data_received += str;
    data_received += "\r\n";

    Serial.print("Received data packet:");
    Serial.println(data_received);
    appendFile("/answers.txt", data_received);

    last_receive_string = "Msg from display " + String(get_display_id() + " was: " + String(str));
    length_to_receive = 0;
}

String get_last_receive_string()
{
    return last_receive_string;
}