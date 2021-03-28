#pragma once
#include "RFV3.h"

#define full_sync_start 0xFE
#define full_sync_end 0xFD

class ModeFullSync : public mode_class
{
public:
  virtual void interrupt()
  {
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
    full_sync_counter = ((get_num_slots() + 1) * 2) - 1;
  }

  virtual void main()
  {
  }

  virtual void post()
  {
    reset_full_sync_count();
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

  void full_sync_pre()
  {
    memset(tx_sync_buffer, 0x00, 7);
    tx_sync_buffer[0] = get_network_id();
    tx_sync_buffer[2] = get_slot_address();
    tx_sync_buffer[1] = full_sync_start;

    printf("Full Sync:");
    for (int i = 0; i < 6; i++)
    {
      printf(" 0x%02x", tx_sync_buffer[i]);
    }
    printf("\r\n");

    cc1101_prepaire_tx(get_freq(), 0);
    full_sync();
    cc1101_tx();
  }

  void full_sync()
  {
    if (tx_sync_buffer[1] == full_sync_end)
    {
      log_normal("full SYNC done");
      cc1101_idle();
      full_sync_counter--;
      if (full_sync_counter < 0 && (millis() - fully_sync_start_time) >= 16000)
        set_mode_idle();
    }
    else
    {
      cc1101_tx_fill(tx_sync_buffer, 7);
      tx_sync_buffer[1]++;
    }
  }
};

ModeFullSync modeFullSync;
