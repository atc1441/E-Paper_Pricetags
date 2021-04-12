#include "settings.h"
#include <FS.h>
#if defined(ESP32)
#include "SPIFFS.h"
#endif
#include "main_variables.h"
#include "cc1101.h"

const char *settings_path = "/settings.txt";

void read_boot_settings()
{
    File myFile;
    String _line_freq, _line_net_id, _line_slots, _line_offset, _line_header;
    if (!SPIFFS.exists(settings_path))
    {
        Serial.println("Error opening the settings file");
        return;
    }
    myFile = SPIFFS.open(settings_path, FILE_READ);
    Serial.print("File size: ");
    Serial.println(myFile.size());

    _line_header = myFile.readStringUntil('\n');
    _line_freq = myFile.readStringUntil('\n');
    _line_net_id = myFile.readStringUntil('\n');
    _line_slots = myFile.readStringUntil('\n');
    _line_offset = myFile.readStringUntil('\n');

    myFile.close();

    uint8_t temp_freq = split(_line_freq, ':', 1).toInt();
    uint8_t temp_net_id = split(_line_net_id, ':', 1).toInt();
    uint8_t temp_slots = split(_line_slots, ':', 1).toInt();
    uint8_t temp_offset = split(_line_offset, ':', 1).toInt();

    Serial.println("Freq read: " + String(temp_freq));
    Serial.println("NetId read: " + String(temp_net_id));
    Serial.println("Slots read: " + String(temp_slots));
    Serial.println("Offset read: " + String(temp_offset));

    set_freq(temp_freq);
    set_network_id(temp_net_id);
    set_num_slot(temp_slots);
    CC1101_set_freq_offset(temp_offset);
}

void save_settings_to_flash()
{
    File myFile;
    myFile = SPIFFS.open(settings_path, FILE_WRITE);

    if (!myFile)
    {
        Serial.println("Error opening file");
        return;
    }

    myFile.print("General settings for ESP32 Pricetag AP\n");
    myFile.printf("CHANNEL:%d:\n", get_freq());
    myFile.printf("NET_ID:%d:\n", get_network_id());
    myFile.printf("SLOTS:%d:\n", (get_num_slots() + 1));
    myFile.printf("FREQ_OFFSET:%d:\n", get_freq_offset());
    myFile.close();
}

void delete_settings_file()
{
    deleteFile(settings_path);
}

void appendFile(const char *path, String message)
{
    Serial.printf("Appending to file: %s\r\n", path);

    File file = SPIFFS.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("- failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- message appended");
    }
    else
    {
        Serial.println("- append failed");
    }
}

void deleteFile(const char *path)
{
    Serial.printf("Deleting file: %s\r\n", path);
    if (SPIFFS.remove(path))
    {
        Serial.println("- file deleted");
    }
    else
    {
        Serial.println("- delete failed");
    }
}

String split(String s, char parser, int index)
{
    String rs = "";
    int parserCnt = 0;
    int rFromIndex = 0, rToIndex = -1;
    while (index >= parserCnt)
    {
        rFromIndex = rToIndex + 1;
        rToIndex = s.indexOf(parser, rFromIndex);
        if (index == parserCnt)
        {
            if (rToIndex == 0 || rToIndex == -1)
                return "";
            return s.substring(rFromIndex, rToIndex);
        }
        else
            parserCnt++;
    }
    return rs;
}
