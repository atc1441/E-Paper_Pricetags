#include <Arduino.h>
#include <SPI.h>
#include "RFV3.h"
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "class.h"
#include "compression.h"
#include "interval_timer.h"
#include "web.h"
#include "utils.h"
#include "trans_assist.h"
#include "arith.h"
#include <FS.h>
#if defined(ESP32)
#include "SPIFFS.h"
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <Hash.h>
#endif

//
// set_bit
//
void set_bit(uint8_t *buffer_bits, int position, bool state)
{
    //set specific bit in byte buffer
    if (buffer_bits != NULL)
    {
        if (state)
            buffer_bits[position / 8] |= 1 << (7 - (position % 8));
        else
            buffer_bits[position / 8] &= ~(1 << (7 - (position % 8)));
    }
} /* set_bit() */
//
// get_bit
//
bool get_bit(uint8_t *buffer_bits, int position)
{
    //get specific bit of byte buffer
    if (buffer_bits[position / 8] & (1 << (7 - (position & 7))))
        return 1;
    else
        return 0;
} /* get_bit() */
//
// get_pixel_in
//
bool get_pixel_in(image_s *image_input, int x, int y)
{
    //get pixel from byte buffer
    if ((x >= 0) && (x < image_input->width) && (y >= 0) && (y < image_input->height))
    {
        return get_bit(&image_input->current_line[1], y);
    }
    return false;
} /* get_pixel_in() */

//
// The key is formed with a 3 pixel group around the current pixel
// C = current pixel, K = key pixels (number = bit number)
//
// +---*+----+
// | K0 | K2 |
// +----+----+
// | K1 | C  |
// +----+----+
//
int get_key_from_pixel(image_s *image_input, int x, int y)
{
    // this function reads the 0 6 7 pixels of the bin_image data
    int key_position = 0;

    //    uint8_t pixel_0, pixel_6, pixel_7;
    int bit, offset;
    uint8_t c;

    offset = y - 1;
    bit = 7 - (offset & 7);
    offset >>= 3;
    // pixel_0
    c = image_input->current_line[1 + offset];
    c = (c >> bit) & 1;
    key_position |= (c << 2);
    // pixel_7
    c = image_input->previous_line[1 + offset];
    key_position |= ((c >> bit) & 1);

    offset = y;
    bit = 7 - (offset & 7);
    offset >>= 3;
    c = image_input->previous_line[1 + offset];
    c = (c >> bit) & 1;
    key_position |= (c << 1);

    return key_position;
} /* get_key_from_pixel() */
//
// get_key_value
//
int get_key_value(image_s *image_input, int x, int y, uint8_t *key_color, entropy_calc_s *entropy)
{
    //reads the key len and key color of the array based on the key_position
    int key_position = get_key_from_pixel(image_input, x, y);

    *key_color = entropy->key_color_out[key_position];
    return entropy->key_len_out[key_position];
} /* get_key_value() */
//
// clean_last_bits
//
void clean_last_bits(encode_data_s *encode_data)
{
    //for every packet bigger then 255 we erase the last bits counting them to one main bit
    int cur_bits_pos = encode_data->out.len - 1;

    if (cur_bits_pos >= 0 && encode_data->out.buffer != NULL)
    {
        while (1)
        {
            if (!get_bit(encode_data->out.buffer, cur_bits_pos))
                break;
            set_bit(encode_data->out.buffer, cur_bits_pos, 0);
            cur_bits_pos--;
            if (cur_bits_pos < 0)
                return;
        }
        set_bit(encode_data->out.buffer, cur_bits_pos, 1);
    }
} /* clean_last_bits() */
//
// write_next_bit
//
void write_next_bit(encode_data_s *encode_data)
{
    //write next bit in output buffer and the needed meta data
    encode_data->cur_part_pixel_count *= 2;
    uint32_t cur_bits_len = encode_data->out.len;

    if (cur_bits_len < encode_data->out.len_max)
    {
        set_bit(encode_data->out.buffer, cur_bits_len, encode_data->countdown_bits & 0x80);
        encode_data->out.len++;
    }
    encode_data->countdown_bits = (encode_data->countdown_bits * 2) & 0xff;
} /* write_next_bit() */
//
// handle_bit_decode
//
void handle_bit_decode(encode_data_s *encode_data, int key_value, int key_color, int pixel_color)
{
    //process the current pixel color and the given key data
    int cur_pixel_count = encode_data->cur_part_pixel_count;
    int trimmed_max_pixel = cur_pixel_count >> key_value;

    if (pixel_color == key_color)
    {
        encode_data->cur_part_pixel_count = trimmed_max_pixel;

        for (int i = key_value; i > 0; i--)
        {
            write_next_bit(encode_data);
        }
    }
    else
    {
        encode_data->countdown_bits += trimmed_max_pixel;
        encode_data->cur_part_pixel_count = cur_pixel_count - trimmed_max_pixel;

        if (encode_data->countdown_bits & 0x100)
        { //for every packet bigger then 255 we erase the last bits counting them to one main bit
            encode_data->countdown_bits = encode_data->countdown_bits & 0xff;
            clean_last_bits(encode_data);
        }

        if (!(encode_data->cur_part_pixel_count & 0x80))
        {
            write_next_bit(encode_data);
        }
    }
} /* handle_bit_decode() */
//
// complete_last_bit_part
//
void complete_last_bit_part(encode_data_s *encode_data)
{
    //when all bits where encoded we need to close the current part, its done here
    for (int i = 0; i < 8; i++)
    {
        write_next_bit(encode_data);
    }
} /* complete_last_bit_part() */
//
// calculate_entropy
//
void calculate_entropy(File file, uint8_t *pBitmap, image_s *bin_image_input, _bmp_s *bmp_infos, entropy_calc_s *calc)
{
    // clear line buffer beyond our image size so that we don't
    // have to check boundaries on every pixel
    memset(&bin_image_input->current_line[0], 0, sizeof(bin_image_input->current_line));

    //go over every pixel and save the average of the len between each key change
    for (int current_x = 0; current_x < bin_image_input->width; current_x++)
    {
        // Current line becomes the previous
        memcpy(bin_image_input->previous_line, bin_image_input->current_line, sizeof(bin_image_input->current_line));
        // Get a new line of image
        // Can be replaced with a "get_src_image_line()" function
        if (pBitmap != NULL) { // memory bitmap
            memcpy(&bin_image_input->current_line[1], &pBitmap[bmp_infos->pitch * current_x], (size_t)bmp_infos->pitch);
        } else {
            if (bmp_infos->bTopDown)
                file.seek(bmp_infos->offset + (bmp_infos->pitch * current_x), SeekSet);
            else
                file.seek(bmp_infos->offset + (bin_image_input->height - 1 - current_x) * bmp_infos->pitch, SeekSet);
            file.read(&bin_image_input->current_line[1], (size_t)bin_image_input->height / 8);
        }

        //memcpy(&bin_image_input->current_line[1], &bin_image_input->input_buffer[current_x * (bin_image_input->height / 8)], bin_image_input->height / 8);

        for (int current_y = 0; current_y < bin_image_input->height; current_y++)
        {

            calc->cur_key_value = get_key_from_pixel(bin_image_input, current_x, current_y);
            calc->cur_key_color = get_pixel_in(bin_image_input, current_x, current_y);

            if ((calc->cur_key_value != calc->last_key_value) && (calc->cur_key_color == calc->last_key_color))
            {

                calc->key_average[calc->last_key_value] = (calc->key_average[calc->last_key_value] + calc->cur_key_counter) / 2;

                calc->cur_key_counter = 1;
                calc->last_key_value = calc->cur_key_value;
            }
            else
            {
                calc->cur_key_counter++;
            }

            calc->last_key_color = calc->cur_key_color;
            //save how many pixels for each key are black or white
            if (calc->cur_key_color)
            {
                calc->color_1_count[calc->cur_key_value]++;
            }
            else
            {
                calc->color_0_count[calc->cur_key_value]++;
            }
        }
    }

    //calculate the color and len average to rounded numbers
    for (int i = 0; i < 8; i++)
    {
        calc->key_len_out[i] = (int)(calc->key_average[i] + 0.5f);
        if (calc->key_len_out[i] < 1)
            calc->key_len_out[i] = 1;
        else if (calc->key_len_out[i] > 7)
            calc->key_len_out[i] = 7;
        calc->key_color_out[i] = (calc->color_0_count[i] > calc->color_1_count[i]);
    }

    // format the key into a byte form for the output in the buffer later
    for (int i = 0; i < 4; i++)
    {
        calc->key[i] = (((calc->key_len_out[i * 2] << 1) | calc->key_color_out[i * 2]) << 4 | ((calc->key_len_out[(i * 2) + 1] << 1) | calc->key_color_out[(i * 2) + 1]));
    }
} /* calculate_entropy() */
//
// encode_raw_image
//
// Pass a NULL value for the output buffer ptr to
// know how big the compressed data will be.
//
// Input data can come from a file or bitmap in memory. if File is NULL
// then the data will be read from the memory pointer and assumed to be a
// top-down bitmap
//
uint32_t encode_raw_image(File file, uint8_t *pBitmap, _bmp_s *bmp_infos, uint8_t *output_bit_buffer, uint32_t max_output_size)
{

    encode_data_s encode_data;
    entropy_calc_s entropy = {0};
    image_s image = {bmp_infos->width, bmp_infos->height};
    uint8_t key_color;

Serial.printf("Entering encode_raw_image; width=%d, height=%d\n", bmp_infos->width, bmp_infos->height);

    if (bmp_infos->height < 1 || bmp_infos->width < 1)
        return 0; //if we dont have any width or heigth there is nothing do encode

    encode_data.out.len = 0;
    encode_data.out.len_max = ((max_output_size - 8) * 8);
    encode_data.out.buffer = NULL;
    encode_data.cur_part_pixel_count = 0xff;
    encode_data.countdown_bits = 0;

    calculate_entropy(file, pBitmap, &image, bmp_infos, &entropy);

    if (output_bit_buffer != NULL)
    {
        output_bit_buffer[0] = 0x03; // entropy description
        output_bit_buffer[1] = 0x00;
        output_bit_buffer[2] = 0x06;
        output_bit_buffer[3] = 0x07;
        output_bit_buffer[4] = entropy.key[0]; // 4 bytes entropy keys and color
        output_bit_buffer[5] = entropy.key[1];
        output_bit_buffer[6] = entropy.key[2];
        output_bit_buffer[7] = entropy.key[3];

        encode_data.out.buffer = &output_bit_buffer[8];
    }

    key_color = 0;

    memset(&image.current_line[0], 0, sizeof(image.current_line));

    for (int current_x = 0; current_x < image.width; current_x++)
    {
        // Current line becomes the previous
        memcpy(image.previous_line, image.current_line, sizeof(image.current_line));
        // Get a new line of image
        // Can be replaced with a "get_src_image_line()" function
        if (pBitmap != NULL) { // memory bitmap
            memcpy(&image.current_line[1], &pBitmap[bmp_infos->pitch * current_x], (size_t)image.height / 8);
        } else {
            if (bmp_infos->bTopDown)
                file.seek(bmp_infos->offset + (bmp_infos->pitch * current_x), SeekSet);
            else
                file.seek(bmp_infos->offset + (bmp_infos->height - 1 - current_x) * bmp_infos->pitch, SeekSet);
            file.read(&image.current_line[1], (size_t)image.height / 8);
        }
        //memcpy(&image.current_line[1], &image.input_buffer[current_x * (image.height / 8)], image.height / 8);

        for (int current_y = 0; current_y < image.height; current_y++)
        {

            uint8_t key_value = get_key_value(&image, current_x, current_y, &key_color, &entropy);

            bool pixel_color = get_pixel_in(&image, current_x, current_y);

            handle_bit_decode(&encode_data, key_value, key_color, pixel_color);
        }
    }
    complete_last_bit_part(&encode_data);

    return 8 /*8bytes for the entropy data*/ + (encode_data.out.len / 8) + ((encode_data.out.len % 8) ? 1 : 0) /*length rounded to next byte*/;
} /* encode_raw_image() */
