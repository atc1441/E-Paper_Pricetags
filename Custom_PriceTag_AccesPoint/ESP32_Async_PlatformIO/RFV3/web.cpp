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
#include <FS.h>
#if defined(ESP32)
#include "SPIFFS.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <OneBitDisplay.h>
#include "font.h"

#include "trans_assist.h"
#include "settings.h"

#include "wlan.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

extern uint8_t data_to_send[];

AsyncWebServer server(80);

//
// Generate custom 1-bpp graphics
//
void GenCustomImage(OBDISP *pOBD, char *text)
{
  int i, j, y, iLen = strlen(text);
  char szTemp[128];

  //Serial.printf("text:%s len = %d\n", text, iLen);
  obdFill(pOBD, 0xff, 1); // colors are inverted
  i = 0;
  y = (pOBD->width >= 480) ? 30 : 0; // starting point for font baseline / top
  while (i < iLen)
  {
    // snip off one line at a time
    j = i; // starting point of this line
    while (j < iLen && (text[j] >= ' '))
    { // search forward for a control char / line break
      j++;
    }
    if (j - i == 0)
      return;
    memcpy(szTemp, &text[i], j - i);
    szTemp[j - i] = 0;
    while (j < iLen && (text[j] < ' '))
    { // skip the ctrl chars
      j++;
    }
    //    Serial.printf("string = %s, at line %d\n", szTemp, y);
    i = j; // ready for next line
    if (pOBD->width == 224 || pOBD->width == 296)
    { // ZBD 50C
      obdWriteString(pOBD, 0, 0, y, szTemp, FONT_12x16, 1, 1);
      y += 2;
    }
    else
    { // all other like: ZBD 900RB & Chroma74
      obdWriteStringCustom(pOBD, (GFXfont *)&Dialog_bold_40, 0, y, szTemp, 0);
      y += 40;
    }
  } // while
} /* GenCustomImage() */

void WriteBMP(const char *filename, uint8_t *pData, int width, int height, int bpp)
{
  int lsize, i, iHeaderSize, iBodySize;
  uint8_t pBuf[128]; // holds BMP header
  File file_out = SPIFFS.open(filename, "wb");

  lsize = (lsize + 3) & 0xfffc; // DWORD aligned
  iHeaderSize = 54;
  iHeaderSize += (1 << (bpp + 2));
  iBodySize = lsize * height;
  i = iBodySize + iHeaderSize; // datasize
  memset(pBuf, 0, 54);
  pBuf[0] = 'B';
  pBuf[1] = 'M';
  pBuf[2] = i & 0xff; // 4 bytes of file size
  pBuf[3] = (i >> 8) & 0xff;
  pBuf[4] = (i >> 16) & 0xff;
  pBuf[5] = (i >> 24) & 0xff;
  /* Offset to data bits */
  pBuf[10] = iHeaderSize & 0xff;
  pBuf[11] = (unsigned char)(iHeaderSize >> 8);
  pBuf[14] = 0x28;
  pBuf[18] = width & 0xff;                // xsize low
  pBuf[19] = (unsigned char)(width >> 8); // xsize high
  i = -height;                            // top down bitmap
  pBuf[22] = i & 0xff;                    // ysize low
  pBuf[23] = (unsigned char)(i >> 8);     // ysize high
  pBuf[24] = 0xff;
  pBuf[25] = 0xff;
  pBuf[26] = 1; // number of planes
  pBuf[28] = (uint8_t)bpp;
  pBuf[30] = 0; // uncompressed
  i = iBodySize;
  pBuf[34] = i & 0xff; // data size
  pBuf[35] = (i >> 8) & 0xff;
  pBuf[36] = (i >> 16) & 0xff;
  pBuf[37] = (i >> 24) & 0xff;
  pBuf[54] = pBuf[55] = pBuf[56] = pBuf[57] = pBuf[61] = 0; // palette
  pBuf[58] = pBuf[59] = pBuf[60] = 0xff;
  {
    uint8_t *s = pData;
    file_out.write(pBuf, iHeaderSize);
    for (i = 0; i < height; i++)
    {
      file_out.write(s, lsize);
      s += lsize;
    }
    file_out.close();
  }
} /* WriteBMP() */

void init_web()
{
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("AutoConnectAP");
  if(!res) {
      Serial.println("Failed to connect");
      ESP.restart();
  } 
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/set_file", HTTP_POST, [](AsyncWebServerRequest *request) {
    int id;
    String filename;
    if (request->hasParam("id") && request->hasParam("file"))
    {
      id = request->getParam("id")->value().toInt();
      filename = request->getParam("file")->value();

      if (!SPIFFS.exists("/" + filename))
      {
        request->send(200, "text/plain", "Error opening file");
        return;
      }

      int size = set_trans_file("/" + filename);
      if (size)
      {
        set_is_data_waiting(id);
      }
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " File: " + filename + " Len: " + String(size));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  // Call custom function to generate a dynamic display
  server.on("/set_custom", HTTP_POST, [](AsyncWebServerRequest *request) {
    int i, id, iSum, iType;
    int width = 0, height = 0;
    OBDISP obd;
    char *pText;
    int comp_size, iSize = 0;
    _bmp_s bmp_info;
    uint8_t *pBitmap, ucCompType = 0;
    String filename = "";
    Serial.println("Entering set_custom");
    if (request->hasParam("id") && request->hasParam("type"))
    {
      id = request->getParam("id")->value().toInt();
      iType = request->getParam("type")->value().toInt();
      Serial.printf("Display type = %d\n", iType);
      if (iType == 0)
      { // 50c
        width = 90;
        height = 224;
      }
      else if (iType == 1)
      { // 900RB
        width = 360;
        height = 480;
      }
      else if (iType == 2)
      { // Chroma29
        width = 128;
        height = 296;
      }
      else if (iType == 3)
      { // Chroma74
        width = 384;
        height = 640;
      }
      //        filename = request->getParam("file")->value();
      filename = "/temp.bin"; // DEBUG
      pText = (char *)request->getParam("text", true)->value().c_str();
      set_display_id(id);
      bmp_info.width = width;
      bmp_info.height = height;
      bmp_info.bsize = (width + 7) / 8;
      bmp_info.pitch = (((height + 7) / 8) + 3) & 0xfffc;
      bmp_info.header_size = (iType == 3) ? 32 : 30;
      pBitmap = &data_to_send[0];
      obdCreateVirtualDisplay(&obd, height, (width + 7) & 0xfff8, pBitmap);
      GenCustomImage(&obd, pText);
      //          for (i=0; i<(width*height/8); i++) { // fake image
      //              pBitmap[i] = (i & 0x200) ? 0xaf : 0x00;
      //          }
      // re-arrange the bytes because the bit/byte layout is different for certain
      if (iType == 1)
      {
        obdCopy(&obd, OBD_MSB_FIRST | OBD_HORZ_BYTES | OBD_ROTATE_90, &data_to_send[32768]);
        iSize = (width / 8) * height;
      }
      else if (iType == 3)
      {                          // Chroma74
        bmp_info.width = height; // swap x/y
        bmp_info.height = width;
        obdCopy(&obd, OBD_MSB_FIRST | OBD_HORZ_BYTES, &data_to_send[32768]);
        iSize = (height / 8) * width;
      }
      else if (iType == 2)
      { // Chroma29
        obdCopy(&obd, OBD_MSB_FIRST | OBD_HORZ_BYTES | OBD_ROTATE_90, &data_to_send[32768]);
        iSize = (width / 8) * height;
      }
      else
      { // 50c
        obdCopy(&obd, OBD_MSB_FIRST | OBD_HORZ_BYTES, &data_to_send[32768]);
        iSize = (height / 8) * width;
      }
      // ============
      // DEBUG
      // Write the uncompressed image to a BMP file to see it
      WriteBMP("/debug.bmp", &data_to_send[32768], bmp_info.width, bmp_info.height, 1);
      // ============
      pBitmap = &data_to_send[32768];
      // calculate uncompressed image checksum
      iSum = 0;
      for (i = 0; i < iSize; i++)
      {
        iSum += pBitmap[i];
      }
      bmp_info.checksum = (uint16_t)iSum;
      // Compress it as RLE or Arithmetic
      if (iType == 0 || iType == 1)
      { // 50c / 900RB
        comp_size = compressBufferRLE(pBitmap, (width * height) / 8, &data_to_send[bmp_info.header_size]);
        ucCompType = 1;
      }
      else if (iType == 3)
      { // Chroma29 + Chroma74
        comp_size = encode_raw_image((File)NULL, pBitmap, &bmp_info, &data_to_send[bmp_info.header_size], 32700);
        ucCompType = 2;
      }
      else
      {
        comp_size = (width * height) / 8;
        memcpy(&data_to_send[bmp_info.header_size],pBitmap,comp_size);
        ucCompType = 0;
      }
      
      iSize = fill_header(data_to_send, comp_size, bmp_info.height, bmp_info.width, ucCompType /* NONE=0, RLE=1, ARITH=2 */, 0 /*colormode*/, bmp_info.header_size, bmp_info.checksum);
      // write it to spiffs
      Serial.printf("Writing %d bytes to temp.bin\n", iSize);
      File file_out = SPIFFS.open(filename, "wb");
      file_out.write(data_to_send, iSize);
      file_out.close();
      iSize = set_trans_file(filename);
      if (iSize)
      {
        set_is_data_waiting(id);
      }
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " File: " + filename + " Len: " + String(iSize));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_bmp_file", HTTP_POST, [](AsyncWebServerRequest *request) {
    int id;
    int iCompressedLen = 0;
    int save_compressed_file_to_spiffs = 0;
    String filename = "";
    String filename_color = "";
    if (request->hasParam("id") && request->hasParam("file"))
    {
      id = request->getParam("id")->value().toInt();
      if (request->hasParam("save_comp_file"))
      {
        save_compressed_file_to_spiffs = (request->getParam("save_comp_file")->value().toInt() ? 1 : 0);
      }
      filename = request->getParam("file")->value();
      if (!SPIFFS.exists("/" + filename))
      {
        request->send(200, "text/plain", "Error opening file");
        return;
      }

      if (request->hasParam("file1"))
      {
        filename_color = request->getParam("file1")->value();
        if (filename_color != "" && !SPIFFS.exists("/" + filename_color))
        {
          request->send(200, "text/plain", "Error opening color file");
          return;
        }
      }
      iCompressedLen = load_img_to_bufer("/" + filename, "/" + filename_color, save_compressed_file_to_spiffs);

      if (iCompressedLen)
      {
        set_is_data_waiting(id);
        request->send(200, "text/plain", "OK cmd to display " + String(id) + " File: " + filename + " Len: " + String(iCompressedLen));
      }
      else
      {
        request->send(200, "text/plain", "Something wrong with the file");
      }
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_cmd", HTTP_POST, [](AsyncWebServerRequest *request) {
    int id;
    String cmd;
    if (request->hasParam("id") && request->hasParam("cmd"))
    {
      id = request->getParam("id")->value().toInt();
      cmd = request->getParam("cmd")->value();
      int cmd_len = cmd.length() / 2;

      uint8_t temp_buffer[cmd_len + 1];
      hexCharacterStringToBytes(temp_buffer, cmd);
      set_trans_buffer(temp_buffer, cmd_len);
      set_is_data_waiting(id);
      set_last_send_status(1);
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " " + cmd + " Len: " + String(cmd_len));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/get_answer", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", get_last_receive_string());
  });

  server.on("/activate_display", HTTP_POST, [](AsyncWebServerRequest *request) {
    String serial;
    int id;
    uint8_t serial_array[7];
    if (request->hasParam("serial") && request->hasParam("id"))
    {
      serial = request->getParam("serial")->value();
      int serial_len = serial.length();
      if (serial_len == 11)
      {
        id = request->getParam("id")->value().toInt();

        set_is_data_waiting(0);

        if (get_trans_mode())
        {
          set_trans_mode(0);
          restore_current_settings();
          set_last_activation_status(0);
          reset_full_sync_count();
        }

        set_display_id(id);

        serial_array[0] = serial[0];
        serial_array[1] = serial[1];
        serial_array[2] = (serial[2] - 0x30) << 4 | (serial[3] - 0x30);
        serial_array[3] = (serial[4] - 0x30) << 4 | (serial[5] - 0x30);
        serial_array[4] = (serial[6] - 0x30) << 4 | (serial[7] - 0x30);
        serial_array[5] = (serial[8] - 0x30) << 4 | (serial[9] - 0x30);
        serial_array[6] = serial[10];

        set_serial(serial_array);

        if (serial_array[6] == 'C')
          set_mode_wun_activation();
        else
          set_mode_wu_activation();

        request->send(200, "text/plain", "OK activating new display " + serial + " to id: " + String(id));
        return;
      }
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/recover_display", HTTP_GET, [](AsyncWebServerRequest *request) {
    String serial;
    uint8_t serial_array[7];
    if (request->hasParam("serial"))
    {
      serial = request->getParam("serial")->value();
      int serial_len = serial.length();
      if (serial_len == 11)
      {

        set_is_data_waiting(0);

        if (get_trans_mode())
        {
          set_trans_mode(0);
          restore_current_settings();
          set_last_activation_status(0);
          reset_full_sync_count();
        }

        serial_array[0] = serial[0];
        serial_array[1] = serial[1];
        serial_array[2] = (serial[2] - 0x30) << 4 | (serial[3] - 0x30);
        serial_array[3] = (serial[4] - 0x30) << 4 | (serial[5] - 0x30);
        serial_array[4] = (serial[6] - 0x30) << 4 | (serial[7] - 0x30);
        serial_array[5] = (serial[8] - 0x30) << 4 | (serial[9] - 0x30);
        serial_array[6] = serial[10];

        set_serial(serial_array);

        set_mode_wu_reset();

        request->send(200, "text/plain", "OK trying to recover display " + serial);
        return;
      }
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/get_mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    String acti_status = "";
    String send_status = "";
    switch (get_last_activation_status())
    {
    case 0:
      acti_status = "not started";
      break;
    case 1:
      acti_status = "started";
      break;
    case 2:
      acti_status = "timeout";
      break;
    case 3:
      acti_status = "successful";
      break;
    default:
      acti_status = "Error";
      break;
    }
    switch (get_last_send_status())
    {
    case 0:
      send_status = "nothing send";
      break;
    case 1:
      send_status = "in sending";
      break;
    case 2:
      send_status = "timeout";
      break;
    case 3:
      send_status = "successful";
      break;
    default:
      send_status = "Error";
      break;
    }

    request->send(200, "text/plain", "Send: " + send_status + " , waiting: " + String(get_is_data_waiting_raw()) + "<br>Activation: " + acti_status + "<br>NetID " + String(get_network_id()) + " freq " + String(get_freq()) + " slot " + String(get_slot_address()) + " bytes left: " + String(get_still_to_send()) + " Open: " + String(get_trans_file_open()) + "<br>last answer: " + get_last_receive_string() + "<br>mode " + get_mode_string());
  });

  server.on("/set_mode", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode"))
    {
      String new_mode = request->getParam("mode")->value();
      if (new_mode == "idle")
      {
        set_is_data_waiting(0);
        set_mode_idle();
      }
      else if (new_mode == "sync")
      {
        set_is_data_waiting(0);
        set_mode_wu();
      }
      else
      {
        return;
      }
      request->send(200, "text/plain", "Ok set mode");
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_id", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("freq") && request->hasParam("net_id"))
    {
      int sniff_freq = request->getParam("freq")->value().toInt();
      int sniff_net_id = request->getParam("net_id")->value().toInt();
      request->send(200, "text/plain", "Ok set IDs Frq: " + String(sniff_freq) + " NetID : " + String(sniff_net_id));
      set_freq(sniff_freq);
      set_network_id(sniff_net_id);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_wu_channel", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("freq"))
    {
      int wu_freq = request->getParam("freq")->value().toInt();
      request->send(200, "text/plain", "Ok set WU Channel: " + String(wu_freq));
      set_wu_channel(wu_freq);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK Reboot");
    ESP.restart();
  });

  server.on("/delete_file", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK delete file");
    deleteFile("/answers.txt");
    deleteFile("/");
  });

  server.on("/get_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "SETTINGS:CHANNEL:" + String(get_freq()) + ":NET_ID:" + String(get_network_id()) + ":SLOTS:" + String(get_num_slots() + 1) + ":FREQ_OFFSET:" + String(get_freq_offset()));
  });

  server.on("/save_settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK saving settings");
    save_settings_to_flash();
  });

  server.on("/delete_settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK delete settings");
    delete_settings_file();
  });

  server.on("/set_num_slot", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("num_slots"))
    {
      int num_slots = request->getParam("num_slots")->value().toInt();
      request->send(200, "text/plain", "Ok set num_slots: " + String(num_slots));
      set_num_slot(num_slots);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_freq_offset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("offset"))
    {
      int freq_offset = request->getParam("offset")->value().toInt();
      request->send(200, "text/plain", "Ok set freq_offset: " + String(freq_offset));
      CC1101_set_freq_offset(freq_offset);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->url() == "/" || request->url() == "index.htm")
    { // not uploaded the index.htm till now so notify the user about it
      request->send(200, "text/html", "please use <a href=\"/edit\">/edit</a> with login defined in wlan.h to uplaod the suplied index.htm to get full useage");
      return;
    }
    Serial.printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
      Serial.printf("GET");
    else if (request->method() == HTTP_POST)
      Serial.printf("POST");
    else if (request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if (request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if (request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if (request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if (request->contentLength())
    {
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }
    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++)
    {
      AsyncWebHeader *h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }
    int params = request->params();
    for (i = 0; i < params; i++)
    {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isFile())
      {
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      }
      else if (p->isPost())
      {
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
      else
      {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(404);
  });

  server.begin();
}
