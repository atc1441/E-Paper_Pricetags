#pragma once
#include "RFV3.h"

#define short_sync_start 0xFE
#define short_sync_end 0x02

class ModeSync : public mode_class
{
  public:
    virtual void interrupt()
    {
      if (after_last_sync == 1) {
        after_last_sync = 2;
        return;
      } else if (after_last_sync == 2) {
        after_last_sync = 0;
        log_normal("short SYNC done");
        cc1101_end_tx();
        if (get_is_data_waiting())
          set_mode_trans();
        else
          set_mode_idle();
        return;
      }
      short_sync();
    }

    virtual void new_interval()
    {
      log_main("something wrong, back to idle");
      set_mode_idle();
    }

    virtual void pre()
    {
      log_main(mode_name);

      after_last_sync = 0;
      CC1101_set_freq(get_freq());
      CC1101_set_net_id(get_network_id());
      memset (tx_sync_buffer, 0x00, 7);
      tx_sync_buffer[0] = get_network_id();
      tx_sync_buffer[2] = get_slot_address();
      tx_sync_buffer[1] = short_sync_start;
      if (get_is_data_waiting()) {
        if(!get_last_to_short())tx_sync_buffer[2] |= 0x80;
        tx_sync_buffer[3] = get_display_id() >> 8;
        tx_sync_buffer[4] = get_display_id() & 0xff;
      }// buffer 5 and 6 is a slot for a second display to contact

      Serial.print("Sync: 0x");
      Serial.print(tx_sync_buffer[0], HEX);
      Serial.print(" 0x");
      Serial.print(tx_sync_buffer[1], HEX);
      Serial.print(" 0x");
      Serial.print(tx_sync_buffer[2], HEX);
      Serial.print(" 0x");
      Serial.print(tx_sync_buffer[3], HEX);
      Serial.print(" 0x");
      Serial.print(tx_sync_buffer[4], HEX);
      Serial.print(" 0x");
      Serial.print(tx_sync_buffer[5], HEX);
      Serial.print(" 0x");
      Serial.println(tx_sync_buffer[6], HEX);

      cc1101_prepaire_tx();
      short_sync();
      set_fifo_interrupt(2, 14);
      cc1101_tx();

    }

    virtual void main()
    {


    }

    virtual void post()
    {
    }

    virtual String get_name()
    {
      return mode_name;
    }

  private:
    String mode_name = "Sync";

    uint8_t tx_sync_buffer[7];
    int after_last_sync = 0;

    void short_sync() {
      send_radio_tx_burst(tx_sync_buffer, 7);
      if (tx_sync_buffer[1] == short_sync_end) {
        set_fifo_interrupt(6, 7);
        after_last_sync = 1;
      }
      tx_sync_buffer[1]++;
    }

};

ModeSync modeSync;
