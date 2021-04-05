#pragma once
#include "RFV3.h"

class ModeWunAct : public mode_class
{
public:
  virtual void interrupt()
  {
    if (handle_wu())
    {
      log(mode_name + " done");

      /* This is the config send to the new Activation version*/
      uint8_t temp_buffer[] = {
          /*SetRfConfig*/ 0x92,
          /*len*/ 0x1D,

          /*Used Frequenzy*/ (uint8_t)(get_freq() + 1),
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          0x00,
          /*NetID temp_network_id*/ get_network_id(),
          /*Display_ID*/ (uint8_t)(get_display_id() & 0xFF), (uint8_t)(get_display_id() >> 8),
          /*Num slots temp_num_slots*/ (uint8_t)(get_num_slots() + 1),
          /*Slot time in MS: 1100*/ 0x4C, 0x04,
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
          /*Max missed sync periods*/ (uint8_t)(60 / (get_num_slots() + 1)),
          /*General config*/ 0x00,
          0x01,
          0x00,

          /*WuSlot*/ (uint8_t)(get_num_slots() & get_display_id()),

          /*GetFirmwareType*/ 0x1F,
          /*GetDisplaySize*/ 0x1A,
          /*DeleteAllImages*/ 0xA1, 0x01, 0xff,

          /*Change RF Group*/ 0xA2, 0x02, 0x01, 0x00,
          /*Set RF Power*/ 0xA4, 0x02, 0xC2, 0xC2};

      uint8_t serial[7];
      get_serial(serial);
      set_display_id(serial[4] << 8 | serial[5]);

      save_current_settings();

      set_num_slot(NEW_ACTIVATION_SLOTS);
      set_freq(NEW_ACTIVATION_FREQ);
      set_network_id(NEW_ACTIVATION_NETID);

      set_trans_buffer(temp_buffer, sizeof(temp_buffer));

      print_buffer(temp_buffer, sizeof(temp_buffer));

      set_is_data_waiting(true);
      set_trans_mode(1);
      set_mode_full_sync();
    }
  }

  virtual void new_interval()
  {
    check_wu_timeout();
  }

  virtual void pre()
  {
    log(mode_name);
    set_last_activation_status(1);
    start_wu(WU_NEW_ACTIVATION_CMD);
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
};

ModeWunAct modeWunAct;
