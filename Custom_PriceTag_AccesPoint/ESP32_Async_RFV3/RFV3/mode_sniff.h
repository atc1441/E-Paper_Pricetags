#pragma once
#include "RFV3.h"

class ModeSniff : public mode_class
{
  public:
    virtual void interrupt()
    {
      read_data_cc1101();
      init_rx();
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
      cc1101_idle();
      set_fifo_interrupt(6, 7);
      cc1101_rx();
    }

    virtual void main()
    {
      if (new_sniff_data) {
        new_sniff_data = false;
        char str[3 * length_to_read];
        tohex(data_array, length_to_read, str, 3 * length_to_read);

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
    }

    virtual String get_name()
    {
      return mode_name;
    }

  private:
    String mode_name = "Sniff";
    uint8_t data_array[255];
    volatile bool new_sniff_data = false;
    uint8_t first_byte, length_to_read = 0;

    String data_received = "";

    void init_rx() {
      cc1101_idle();
      cc1101_rx();
    }

    void read_data_cc1101() {


      first_byte = spi_read_register(0x3B);
      length_to_read = spi_read_register(0x3F);

      if (length_to_read >= 63) {
        Serial.println("");
        Serial.println("data to long");
        return;
      }

      spi_read_burst(0xFF, data_array, length_to_read);

      switch (length_to_read) {
        default:
          break;
        case 7://Sync
          Serial.println(data_array[1], HEX);
          break;
        case 10://Wakeup
          break;
        case 16://V2 Activation
          break;
        case 62://Actual Data
          Serial.println(data_array[4], HEX);
          break;
      }


      set_buffer_length_sniff(length_to_read);
      copy_buffer_sniff(data_array, length_to_read);

      /*
            Serial.print(" Data: ");
            for (int i = 0; i < length_to_read; i++) {
              Serial.print(" 0x");
              Serial.print(data_array[i], HEX);
            }*/
      new_sniff_data = true;

      /*    Serial.print(" Data2: ");
          for (int i = 0; i < 0x02; i++) {
            Serial.print(" 0x");
            Serial.print(data_array1[i], HEX);
          }
          Serial.println();
      */
    }

};

ModeSniff modeSniff;
