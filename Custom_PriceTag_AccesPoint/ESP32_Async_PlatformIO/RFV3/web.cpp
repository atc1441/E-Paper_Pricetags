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
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include "trans_assist.h"
#include "settings.h"

#include "wlan.h"

AsyncWebServer server(80);

void init_web()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  WiFi.softAP("home", "aaaaaaaa");
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }
  long start_wifi = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (millis() - start_wifi > 20000)
      return;
    if (WiFi.softAPgetStationNum() > 0)
      return;
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/set_file", HTTP_GET, [](AsyncWebServerRequest *request) {
    int id;
    String filename;
    if (request->hasParam("id") && request->hasParam("file"))
    {
      id = request->getParam("id")->value().toInt();
      filename = request->getParam("file")->value();

      int size = set_trans_file("/" + filename);
      if (size)
      {
        set_display_id(id);
        set_is_data_waiting(true);
        set_last_send_status(1);
      }
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " File: " + filename + " Len: " + String(size));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_bmp_file", HTTP_GET, [](AsyncWebServerRequest *request) {
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

      if (request->hasParam("file1"))
        filename_color = request->getParam("file1")->value();

      iCompressedLen = load_img_to_bufer("/" + filename, "/" + filename_color, save_compressed_file_to_spiffs);

      if (iCompressedLen)
      {
        set_display_id(id);
        set_is_data_waiting(true);
        set_last_send_status(1);
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

  server.on("/set_cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
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
      set_is_data_waiting(true);
      set_display_id(id);
      set_last_send_status(1);
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " " + cmd + " Len: " + String(cmd_len));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/get_answer", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", get_last_receive_string());
  });

  server.on("/activate_display", HTTP_GET, [](AsyncWebServerRequest *request) {
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

        set_is_data_waiting(false);

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

        set_is_data_waiting(false);

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

    request->send(200, "text/plain", "Send: " + send_status + " , Activation: " + acti_status + " NetID " + String(get_network_id()) + " freq " + String(get_freq()) + " slot " + String(get_slot_address()) + " bytes left: " + String(get_still_to_send()) + " Open: " + String(get_trans_file_open()) + " is waiting: " + String(get_is_data_waiting_raw()) + "<br>mode " + get_mode_string());
  });

  server.on("/set_mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode"))
    {
      String new_mode = request->getParam("mode")->value();
      if (new_mode == "idle")
      {
        set_mode_idle();
      }
      else if (new_mode == "sync")
      {
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

  server.on("/set_id", HTTP_GET, [](AsyncWebServerRequest *request) {
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

  server.on("/set_wu_channel", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("freq"))
    {
      int wu_freq = request->getParam("freq")->value().toInt();
      request->send(200, "text/plain", "Ok set WU Channel: " + String(wu_freq));
      set_wu_channel(wu_freq);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK Reboot");
    ESP.restart();
  });

  server.on("/delete_file", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK delete file");
    deleteFile("/answers.txt");
    deleteFile("/");
  });

  server.on("/get_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "SETTINGS:CHANNEL:" + String(get_freq()) + ":NET_ID:" + String(get_network_id()) + ":SLOTS:" + String(get_num_slots() + 1) + ":FREQ_OFFSET:" + String(get_freq_offset()));
  });

  server.on("/save_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK saving settings");
    save_settings_to_flash();
  });

  server.on("/delete_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK delete settings");
    delete_settings_file();
  });

  server.on("/set_num_slot", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("num_slots"))
    {
      int num_slots = request->getParam("num_slots")->value().toInt();
      request->send(200, "text/plain", "Ok set num_slots: " + String(num_slots));
      set_num_slot(num_slots);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_freq_offset", HTTP_GET, [](AsyncWebServerRequest *request) {
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
