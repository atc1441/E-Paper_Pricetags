#pragma once
#include "RFV3.h"

class ModeSniff : public mode_class
{
public:
  virtual void interrupt()
  {
    read_data_cc1101();
    cc1101_rx();
  }

  virtual void new_interval()
  {
  }

  virtual void pre()
  {
    log_main(mode_name);
    data_received = "\r\nStart\r\nFreq: ";
    data_received += String(get_sniff_freq());
    data_received += " ID: ";
    data_received += String(get_sniff_net_id());
    data_received += "\r\n";
    CC1101_set_freq(get_sniff_freq());
    CC1101_set_net_id(get_sniff_net_id());
    cc1101_rx();
  }

  virtual void main()
  {
    if (new_sniff_data)
    {
      new_sniff_data = false;
      char str[3 * read_len];
      tohex(data_array, read_len, str, 3 * read_len);

      data_received += String(millis());
      data_received += " = ";
      data_received += str;
      data_received += "\r\n";
      Serial.println("Len: " + String(data_received.length()));
    }
  }

  virtual void post()
  {
    appendFile("/sniff.txt", data_received);
    cc1101_idle();
  }

  virtual String get_name()
  {
    return mode_name;
  }

private:
  String mode_name = "Sniff";
  uint8_t data_array[255];
  volatile bool new_sniff_data = false;
  int read_len = 0;

  String data_received = "";

  bool read_data_cc1101()
  {
    read_len = cc1101_read_fifo(data_array);

    if (read_len == -1)
    {
      Serial.println("Error while reading RX buffer");
      return false;
    }
    
    Serial.print(" Read_len:" + String(read_len-1));
    Serial.print(" Data:");
    for (int i = 0; i < read_len; i++)
    {
      Serial.print(" 0x");
      Serial.print(data_array[i], HEX);
    }
    Serial.println();

    switch (read_len - 1)
    {
    default:
      break;
    case 7: //Sync
      Serial.println(data_array[1], HEX);
      break;
    case 10: //Wakeup
      break;
    case 16: //V2 Activation
      break;
    case 62: //Actual Data
      Serial.println(data_array[4], HEX);
      break;
    }

    set_buffer_length_sniff(read_len);
    copy_buffer_sniff(data_array, read_len);

    new_sniff_data = true;
    return true;
  }
};

ModeSniff modeSniff;
