#pragma once
#include "RFV3.h"

#define short_sync_start 0xFE
#define short_sync_end 0x02

class ModeSync : public mode_class
{
public:
  virtual void interrupt()
  {
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

    memset(tx_sync_buffer, 0x00, 7);
    tx_sync_buffer[0] = get_network_id();
    tx_sync_buffer[2] = get_slot_address();
    tx_sync_buffer[1] = short_sync_start;
    if (get_is_data_waiting())
    {
      if (!get_last_to_short())
        tx_sync_buffer[2] |= 0x80;
      tx_sync_buffer[3] = get_display_id() >> 8;
      tx_sync_buffer[4] = get_display_id() & 0xff;
    } // buffer 5 and 6 is a slot for a second display to contact but not used here

    printf("Sync:");
    for (int i = 0; i < 6; i++)
    {
      printf(" 0x%02x", tx_sync_buffer[i]);
    }
    printf("\r\n");

    cc1101_prepaire_tx(get_freq(), get_network_id());
    short_sync();
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

  void short_sync()
  {
    if (tx_sync_buffer[1] == short_sync_end)
    {
      log_normal("short SYNC done");
      cc1101_idle();
      if (check_trans_mode_last())
      {
        return;
      }
      if (get_is_data_waiting())
        set_mode_trans();
      else
        set_mode_idle();
    }
    else
    {
      cc1101_tx_fill(tx_sync_buffer, 7);
      tx_sync_buffer[1]++;
    }
  }
};

ModeSync modeSync;
