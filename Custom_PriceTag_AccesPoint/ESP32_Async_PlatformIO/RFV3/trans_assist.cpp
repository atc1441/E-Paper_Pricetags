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
#include "arith.h"
#include "settings.h"

#define _receive_size 0x1000
#define _send_size 65535

File file;
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

    last_receive_string = "Msg from display " + data_received;
    length_to_receive = 0;
}

String get_last_receive_string()
{
    return last_receive_string;
}

int get_compress_method_from_size(int width, int height, int *big_address)
{ // 2= Arith, 1=RLE, 0=No
    *big_address = 0;
    if (width == 640 && height == 384) // Chroma74
    {
        *big_address = 1;
        return 2;
    }
    else if (width == 400 && height == 300) // Chroma42
    {
        return 2;
    }
    else if (width == 128 && height == 296) // Chroma29
    {
        return 2;
    }
    else if (width == 224 && height == 90) // Epop50
    {
        return 1;
    }
    else if (width == 320 && height == 240) // Epop500
    {
        return 2;
    }
    else if (width == 360 && height == 480) // Epop900
    {
        return 1;
    }
    else
        return 0;
}

uint8_t open_bmp(String &path, _bmp_s *bmp_infos)
{
    file = SPIFFS.open(path, "rb");
    if (file == 0)
    {
        return 1;
    }

    uint8_t bmp_header[0x30] = {0};
    file.read(bmp_header, (size_t)0x30);

    if (bmp_header[0] != 'B' || bmp_header[1] != 'M' || bmp_header[14] < 0x28)
    {
        file.close();
        Serial.println("Not a Windows BMP file!\n");
        return 0;
    }
    bmp_infos->width = *(int32_t *)&bmp_header[18];
    bmp_infos->height = *(int32_t *)&bmp_header[22];
    bmp_infos->offset = *(int32_t *)&bmp_header[10];
    bmp_infos->bsize = (bmp_infos->width + 7) / 8;
    bmp_infos->pitch = (bmp_infos->bsize + 3) & 0xfffc;

    bmp_infos->bTopDown = 1;
    if (bmp_infos->height > 0)
    {
        bmp_infos->bTopDown = 0;
    }
    else
    {
        bmp_infos->height = 0 - bmp_infos->height;
    }
    printf("input bitmap size: %d x %d\n", bmp_infos->width, bmp_infos->height);
    return 0;
}

int load_img_to_bufer(String &path, String &path1, bool save_file_to_spiffs)
{
    reset_trans_values();
    _bmp_s bmp_infos;
    if (open_bmp(path, &bmp_infos))
        return 0;
    file_is_open = 1;
    int comp_size = 0;

    int colormode = 0;
    File file_color;
    if (path1 != "/")
    {
        colormode = 1;
        file_color = SPIFFS.open(path1, "rb");
        if (!file_color)
            colormode = 0;
    }
    int big_address = 0;
    int compression_mode = get_compress_method_from_size(bmp_infos.width, bmp_infos.height, &big_address);
    bmp_infos.header_size = (big_address ? 32 : 30);

    if (compression_mode == 2) //Arith
    {
        comp_size = encode_raw_image(file, &bmp_infos, &data_to_send[bmp_infos.header_size], _send_size - 32 - 7);
        if (colormode)
            comp_size += encode_raw_image(file_color, &bmp_infos, &data_to_send[bmp_infos.header_size + comp_size], _send_size - 7 - comp_size);
        if (comp_size <= 0 && comp_size > ((bmp_infos.width + bmp_infos.height) / 8) * (colormode ? 2 : 1))
        { // if compression failed or to big do none compression
            compression_mode = 0;
        }
    }
    else if (compression_mode == 1) //RLE
    {
        //TODO after RLE fully implemented remove compression mode change here!!!!
        compression_mode = 0; //<-----THIS ONE
                              /* comp_size = load_img_to_bufer_rle(file, &bmp_infos);
        if (colormode)
            comp_size += load_img_to_bufer_rle(file_color, &bmp_infos);
        if (comp_size <= 0 && comp_size > ((bmp_infos.width + bmp_infos.height) / 8) * (colormode ? 2 : 1))
        { // if compression failed or to big do none compression
            compression_mode = 0;
        }*/
    }

    if (compression_mode == 0)
    {
        comp_size = load_img_to_bufer_none(file, &bmp_infos);
        if (colormode)
            comp_size += load_img_to_bufer_none(file_color, &bmp_infos);
    }

    if (colormode)
        file_color.close();
    file.close();
    file_is_open = 0;

    length_to_send = fill_header(data_to_send, comp_size, bmp_infos.height, bmp_infos.width, compression_mode, colormode, bmp_infos.header_size, bmp_infos.checksum);

    if (save_file_to_spiffs)
    {
        File file_log = SPIFFS.open("/last_compressed_img.bin", "wb");
        file_log.write(data_to_send, length_to_send);
        file_log.close();
    }
    return length_to_send;
}

int load_img_to_bufer_none(File file_in, _bmp_s *bmp_infos)
{
    int comp_size = 0;

    for (int y = 0; y < bmp_infos->height; y++)
    {
        if (bmp_infos->bTopDown)
            file_in.seek(bmp_infos->offset + (bmp_infos->pitch * y), SeekSet);
        else
            file_in.seek(bmp_infos->offset + (bmp_infos->height - 1 - y) * bmp_infos->pitch, SeekSet);
        file_in.read(&data_to_send[bmp_infos->header_size + comp_size], (size_t)bmp_infos->bsize);
        for (int x = 0; x < bmp_infos->bsize; x++)
        {
            bmp_infos->checksum = bmp_infos->checksum + data_to_send[bmp_infos->header_size + comp_size + x];
        }
        comp_size += bmp_infos->bsize;
    }
    return comp_size;
}

int load_img_to_bufer_rle(File file_in, _bmp_s *bmp_infos)
{
    /*NOT FINISCHED its just none right now :D*/
    int comp_size = 0;

    for (int y = 0; y < bmp_infos->height; y++)
    {
        if (bmp_infos->bTopDown)
            file_in.seek(bmp_infos->offset + (bmp_infos->pitch * y), SeekSet);
        else
            file_in.seek(bmp_infos->offset + (bmp_infos->height - 1 - y) * bmp_infos->pitch, SeekSet);
        file_in.read(&data_to_send[bmp_infos->header_size + comp_size], (size_t)bmp_infos->bsize);
        for (int x = 0; x < bmp_infos->bsize; x++)
        {
            bmp_infos->checksum = bmp_infos->checksum + data_to_send[bmp_infos->header_size + comp_size + x];
        }
        comp_size += bmp_infos->bsize;
    }
    return comp_size;
}

int fill_header(uint8_t *buffer_out, int compression_size, int height, int width, int compression_type, int color, int header_size, uint16_t checksum)
{
    // fill the output data so it can directly be send to the display
    buffer_out[0] = 0x83;
    buffer_out[1] = 0x19;
    buffer_out[2] = 0;
    buffer_out[3] = (uint8_t)width;
    buffer_out[4] = (uint8_t)(width >> 8);
    buffer_out[5] = (uint8_t)height;
    buffer_out[6] = (uint8_t)(height >> 8);
    buffer_out[7] = (uint8_t)(compression_size >> 0);
    buffer_out[8] = (uint8_t)(compression_size >> 8);

    buffer_out[22] = (uint8_t)(0x100 - checksum);
    buffer_out[23] = compression_type;

    buffer_out[26] = color ? 0x21 : 0x00; // 0 for one color 21 for two color

    buffer_out[27] = 0x84;

    if (header_size == 32)
    {
        buffer_out[28] = 0x80;
        buffer_out[29] = 0x00;
        buffer_out[30] = (uint8_t)(compression_size >> 8);
        buffer_out[31] = (uint8_t)(compression_size >> 0);
    }
    else
    {
        buffer_out[28] = (uint8_t)(compression_size >> 8) | 0x80;
        buffer_out[29] = (uint8_t)(compression_size >> 0);
    }
    buffer_out[compression_size + header_size + 0] = 0x85;
    buffer_out[compression_size + header_size + 1] = 0x05;
    buffer_out[compression_size + header_size + 2] = 0x08;
    buffer_out[compression_size + header_size + 3] = 0x00;
    buffer_out[compression_size + header_size + 4] = 0x00;
    buffer_out[compression_size + header_size + 5] = 0x01;
    buffer_out[compression_size + header_size + 6] = 0x01;

    return header_size + compression_size + 7;
}
