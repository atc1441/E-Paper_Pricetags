#include <Arduino.h>
#include <SPI.h>
#include "RFV3.h"
#include "main_variables.h"
#include "logger.h"
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

#include "wlan.h"

AsyncWebServer server(80);

void init_web() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  WiFi.softAP("home", "aaaaaaaa");
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }
  long start_wifi = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start_wifi > 20000)return;
    if (WiFi.softAPgetStationNum() > 0)return;
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  SPIFFS.begin(true);

  server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  

  server.on("/add_display", HTTP_GET, [] (AsyncWebServerRequest * request) {
    int id;
    String cmd;
    if (request->hasParam("id") && request->hasParam("serial")) {
      id = request->getParam("id")->value().toInt();
      cmd = request->getParam("serial")->value();
      int cmd_len = cmd.length() / 2;
      set_display_id(id);

      uint8_t temp_buffer[cmd_len + 1];
      hexCharacterStringToBytes(temp_buffer, cmd);
      set_buffer_length(cmd_len);
      set_data_to_send(temp_buffer, cmd_len);
      set_is_data_waiting(true);
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " " + cmd + " Len: " + String(cmd_len));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });
  

  server.on("/set_file", HTTP_GET, [] (AsyncWebServerRequest * request) {
    int id;
    String filename;
    if (request->hasParam("id") && request->hasParam("file")) {
      id = request->getParam("id")->value().toInt();
      filename = request->getParam("file")->value();

      set_display_id(id);

      File file = SPIFFS.open("/" + filename, "rb");
      if (file == 0) {
        request->send(200, "text/plain", "Could not open file");
        return;
      }

      long size;
      file.seek(0, SeekEnd);
      size = file.position();
      file.seek(0, SeekSet);

      unsigned char *buffer = (unsigned char *)malloc((unsigned long)size);
      if (buffer == NULL) {
        file.close();
        request->send(200, "text/plain", "Could not malloc buffer");
        return;
      }

      file.read(buffer, (size_t)size);
      file.close();


      set_buffer_length(size);
      set_data_to_send(buffer, size);
      set_is_data_waiting(true);
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " File: " + filename + " Len: " + String(size));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_bmp_file", HTTP_GET, [] (AsyncWebServerRequest * request) {
    int id;
    int compression;
    int iCompressedLen;
    String filename;
    if (request->hasParam("id") && request->hasParam("file") && request->hasParam("comp")) {
      id = request->getParam("id")->value().toInt();
      filename = request->getParam("file")->value();
      compression = request->getParam("comp")->value().toInt();

      set_display_id(id);

      File file = SPIFFS.open("/" + filename, "rb");
      if (file == 0) {
        request->send(200, "text/plain", "Could not open file");
        return;
      }

      long size;
      file.seek(0, SeekEnd);
      size = file.position();
      file.seek(0, SeekSet);

      unsigned char *buffer = (unsigned char *)malloc((unsigned long)size);
      if (buffer == NULL) {
        file.close();
        request->send(200, "text/plain", "Could not malloc buffer");
        return;
      }
      file.read(buffer, (size_t)size);
      file.close();
      
      if (compression == 0) {
        iCompressedLen = compressImageNONE(buffer, 0, size);
      } else if (compression == 1) {
        iCompressedLen = compressImageRLE(buffer, 0, size);
      } else if (compression == 2) {
        request->send(200, "text/plain", "Arithmetic comopression is not supported right now");
        return;
      } else {
        request->send(200, "text/plain", "No valid compression supplied");
        return;
      }

      set_buffer_length(iCompressedLen);
      set_data_to_send(buffer, iCompressedLen);
      set_is_data_waiting(true);
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " File: " + filename + " Len: " + String(iCompressedLen));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_cmd", HTTP_GET, [] (AsyncWebServerRequest * request) {
    int id;
    String cmd;
    if (request->hasParam("id") && request->hasParam("cmd")) {
      id = request->getParam("id")->value().toInt();
      cmd = request->getParam("cmd")->value();
      int cmd_len = cmd.length() / 2;
      set_display_id(id);

      uint8_t temp_buffer[cmd_len + 1];
      hexCharacterStringToBytes(temp_buffer, cmd);
      set_buffer_length(cmd_len);
      set_data_to_send(temp_buffer, cmd_len);
      set_is_data_waiting(true);
      request->send(200, "text/plain", "OK cmd to display " + String(id) + " " + cmd + " Len: " + String(cmd_len));
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/get_answer", HTTP_GET, [] (AsyncWebServerRequest * request) {
    char temp_buffer[(get_buffer_length_answer() * 3) + 1];
    uint8_t temp_buffer_byte[get_buffer_length_answer() + 1];
    get_data_to_send(temp_buffer_byte, get_buffer_length_answer());

    tohex(temp_buffer_byte, get_buffer_length_answer(), temp_buffer, 3 * get_buffer_length_answer());

    request->send(200, "text/plain",  temp_buffer);
  });

  server.on("/activate_display", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String serial;
    int id;
    uint8_t serial_array[7];
    if (request->hasParam("serial") && request->hasParam("id")) {
      serial = request->getParam("serial")->value();
      int serial_len = serial.length();
      if (serial_len == 11) {
        id = request->getParam("id")->value().toInt();
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

  server.on("/get_last_activation", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String act_status = "has failed";
    if (get_last_activation_status()) act_status = "was successful";
    request->send(200, "text/plain", "Last activation " + act_status);
  });

  server.on("/get_mode", HTTP_GET, [] (AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Current mode " + get_mode_string());
  });

  server.on("/set_mode", HTTP_GET, [] (AsyncWebServerRequest * request) {
    if (request->hasParam("mode")) {
      String new_mode = request->getParam("mode")->value();
      if (new_mode == "idle") {
        set_mode_idle();
      } else if (new_mode == "sniff") {
        set_mode_sniff();
      } else if (new_mode == "sync") {
        set_mode_wu();
      } else {
        return;
      }
      request->send(200, "text/plain", "Ok set mode");
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_sniff", HTTP_GET, [] (AsyncWebServerRequest * request) {
    if (request->hasParam("freq") && request->hasParam("net_id")) {
      int sniff_freq = request->getParam("freq")->value().toInt();
      int sniff_net_id = request->getParam("net_id")->value().toInt();
      request->send(200, "text/plain", "Ok set sniff Frq: " + String(sniff_freq) + " NetID : " + String(sniff_net_id));
      set_sniff_freq(sniff_freq);
      set_sniff_net_id(sniff_net_id);
      set_mode_sniff();
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/get_sniff", HTTP_GET, [] (AsyncWebServerRequest * request) {
    char temp_buffer[(get_buffer_length_sniff() * 3) + 1];
    uint8_t temp_buffer_byte[get_buffer_length_sniff() + 1];
    get_data_to_send_sniff(temp_buffer_byte, get_buffer_length_sniff());

    tohex(temp_buffer_byte, get_buffer_length_sniff(), temp_buffer, 3 * get_buffer_length_sniff());

    request->send(200, "text/plain",  temp_buffer);
  });

  server.on("/set_id", HTTP_GET, [] (AsyncWebServerRequest * request) {
    if (request->hasParam("freq") && request->hasParam("net_id")) {
      int sniff_freq = request->getParam("freq")->value().toInt();
      int sniff_net_id = request->getParam("net_id")->value().toInt();
      request->send(200, "text/plain", "Ok set IDs Frq: " + String(sniff_freq) + " NetID : " + String(sniff_net_id));
      set_freq(sniff_freq);
      set_network_id(sniff_net_id);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.on("/set_wu_channel", HTTP_GET, [] (AsyncWebServerRequest * request) {
    if (request->hasParam("freq")) {
      int wu_freq = request->getParam("freq")->value().toInt();
      request->send(200, "text/plain", "Ok set WU Channel: " + String(wu_freq));
      set_wu_channel(wu_freq);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });
  server.on("/delete_file", HTTP_GET, [] (AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK delete sniff file");
    deleteFile("/sniff.txt");
    deleteFile("/answers.txt");
    deleteFile("/answers_n.txt");
  });

  server.on("/set_num_slot", HTTP_GET, [] (AsyncWebServerRequest * request) {
    if (request->hasParam("num_slots")) {
      int num_slots = request->getParam("num_slots")->value().toInt();
      request->send(200, "text/plain", "Ok set num_slots: " + String(num_slots));
      set_num_slot(num_slots);
      return;
    }
    request->send(200, "text/plain", "Wrong parameter");
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.onNotFound([](AsyncWebServerRequest * request) {
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

    if (request->contentLength()) {
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }
    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++) {
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }
    int params = request->params();
    for (i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isFile()) {
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if (p->isPost()) {
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(404);
  });

  server.begin();
}


void appendFile(const char * path, String message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = SPIFFS.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
}

void deleteFile(const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (SPIFFS.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}
