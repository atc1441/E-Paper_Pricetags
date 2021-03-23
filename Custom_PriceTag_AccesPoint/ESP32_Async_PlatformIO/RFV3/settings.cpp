#include "settings.h"
#include <FS.h>
#include <SPIFFS.h>

File myFile;
const char *settings_path = "/settings.txt";

void read_boot_settings()
{
    String s, p;
    if (!SPIFFS.exists(settings_path))
    {
        Serial.println("Error opening the settings file");
        return;
    }
    myFile = SPIFFS.open(settings_path, FILE_READ);
    Serial.print("File size: ");
    Serial.println(myFile.size());
    s = myFile.readStringUntil('\n');
    p = myFile.readStringUntil('\n');
    myFile.close();

    Serial.println("S1: " + split(s, ':',0));
    Serial.println("S2: " + split(s, ':',1));
    Serial.println("P1: " + split(p, ':',0));
    Serial.println("P2: " + split(p, ':',1));
}

void write_setting(String setting, String state)
{
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
    int parserIndex = index;
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
