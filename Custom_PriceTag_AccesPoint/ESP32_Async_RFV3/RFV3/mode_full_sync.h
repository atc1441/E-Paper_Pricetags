#pragma once
#include "RFV3.h"

#define full_sync_start 0xFE
#define full_sync_end 0xFD

class ModeFullSync : public mode_class
{
  public:
    virtual void interrupt()
    {
      if (after_last_sync == 1) {
        after_last_sync = 2;
        return;
      } else if (after_last_sync == 2) {
        after_last_sync = 0;
        log_normal("full SYNC done");
        cc1101_end_tx();
        full_sync_counter--;
        if (full_sync_counter < 0 && (millis() - fully_sync_start_time) >= 16000)
          set_mode_idle();
        return;
      }
      full_sync();
    }

    virtual void new_interval()
    {
      full_sync_pre();
    }

    virtual void pre()
    {
      log_main(mode_name);
      fully_sync_start_time = millis();
      full_sync_counter = ((get_num_slots() + 1) * 3) - 1;
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
    String mode_name = "Full Sync";

    int full_sync_counter = 0;
    long fully_sync_start_time = 0;

    uint8_t tx_sync_buffer[7];

    int after_last_sync = 0;

    void full_sync_pre() {
      after_last_sync = 0;
      memset (tx_sync_buffer, 0x00, 7);
      tx_sync_buffer[0] = get_network_id();
      tx_sync_buffer[2] = get_slot_address();
      CC1101_set_freq(get_freq());
      tx_sync_buffer[1] = full_sync_start;

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
      full_sync();
      set_fifo_interrupt(2, 14);
      cc1101_tx();
    }

    void full_sync() {
      send_radio_tx_burst(tx_sync_buffer, 7);
      if (tx_sync_buffer[1] == full_sync_end) {
        set_fifo_interrupt(6, 7);
        after_last_sync = 1;
        return;
      }
      tx_sync_buffer[1]++;
    }

};

ModeFullSync modeFullSync;
