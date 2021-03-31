#pragma once
#include "RFV3.h"

#define NEW_ACTIVATION_SLOTS 4
#define NEW_ACTIVATION_FREQ 8
#define NEW_ACTIVATION_NETID 254

class ModeWunAct : public mode_class
{
public:
  virtual void interrupt()
  {
    wakeup();
  }

  virtual void new_interval()
  {
  }

  virtual void pre()
  {
    log_main(mode_name);
    set_last_activation_status(1);

    wakeup_start_time = millis();

    memset(tx_wu_buffer, 0xff, 10);
    tx_wu_buffer[0] = 0x00;
    tx_wu_buffer[1] = 0xff;
    tx_wu_buffer[2] = 0x05;                    // Num Periods per slot
    tx_wu_buffer[3] = 0x4c;                    // Slot time in MS LOW
    tx_wu_buffer[4] = 0x04;                    // Slot time in MS HIGH
    tx_wu_buffer[5] = NEW_ACTIVATION_SLOTS;    // Num Slots Activation
    tx_wu_buffer[6] = 0x02;                    // maybe max missed syncs
    tx_wu_buffer[7] = NEW_ACTIVATION_FREQ + 1; // Frequenzy
    tx_wu_buffer[8] = 0x03;                    // CMD mode = New Activation
    tx_wu_buffer[9] = NEW_ACTIVATION_NETID;    // Used NetID for Activation

    printf("Wun Activation:");
    for (int i = 0; i < 9; i++)
    {
      printf(" 0x%02x", tx_wu_buffer[i]);
    }
    printf("\r\n");

    cc1101_prepaire_tx(get_wu_channel(), 0);
    wakeup();
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
  String mode_name = "Wakeup new Activation";

  long wakeup_start_time;
  uint8_t tx_wu_buffer[10];

  void wakeup()
  {
    if (millis() - wakeup_start_time > 15500)
    {
      uint8_t temp_freq = get_freq();
      uint8_t temp_network_id = get_network_id();
      uint16_t temp_display_id = get_display_id();
      uint8_t temp_num_slots = get_num_slots() + 1;
      uint8_t temp_display_slot = temp_num_slots & temp_display_id;

      save_current_settings();

      set_num_slot(NEW_ACTIVATION_SLOTS);
      set_freq(NEW_ACTIVATION_FREQ);
      set_network_id(NEW_ACTIVATION_NETID);

      uint8_t serial[7];
      get_serial(serial);
      set_display_id(serial[4] << 8 | serial[5]);

      /* This is the config send to the new Activation version*/
      uint8_t temp_buffer[] = {
          /*SetRfConfig*/ 0x92,
          /*len*/ 0x1D,

          /*Used Frequenzy*/ (uint8_t)(temp_freq + 1),

          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,

          /*NetID temp_network_id*/ temp_network_id,

          /*Display_ID*/ (uint8_t)(temp_display_id & 0xFF),
          (uint8_t)(temp_display_id >> 8),

          /*Num slots temp_num_slots*/ (uint8_t)temp_num_slots,

          /*Slot time in MS: 1100*/ 0x4C,
          0x04,

          0x00,
          0x00,
          0xFF,
          0x00,

          /*Periods per slot sync???*/ 0x05,

          /*Freq Location Region???*/ 0x40,

          0x18,
          0xFC,
          0xE7,
          0x03,

          /*Max missed sync periods*/ (uint8_t)(60 / temp_num_slots),

          /*General config*/ 0x00,
          0x01,
          0x00,

          /*WuSlot*/ temp_display_slot,

          /*GetFirmwareType*/ 0x1F,
          /*GetDisplaySize*/ 0x1A,
          /*DeleteAllImages*/ 0xA1, 0x01, 0xff};

      set_trans_buffer(temp_buffer, sizeof(temp_buffer));

      set_is_data_waiting(true);
      set_trans_mode(1);
      set_mode_full_sync();
    }
    else
    {
      cc1101_tx_fill(tx_wu_buffer, 10);
    }
  }
};

ModeWunAct modeWunAct;
