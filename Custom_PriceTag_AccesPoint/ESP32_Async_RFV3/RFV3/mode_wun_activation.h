#pragma once
#include "RFV3.h"

class ModeWunAct : public mode_class
{
  public:
    virtual void interrupt()
    {
      if (last_wu_act) {
        uint8_t temp_freq = get_freq();
        uint8_t temp_network_id = get_network_id();
        uint16_t temp_display_id = get_display_id();
        uint8_t temp_num_slots = get_num_slots();
        uint8_t temp_display_slot = temp_num_slots & temp_display_id;

        save_current_settings();

        set_num_slot(0x10);
        set_freq(0);
        set_network_id(0xfe);


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

          /*Num slots temp_num_slots*/ (uint8_t)(temp_num_slots + 1),

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

          /*Max missed sync periods*/ (uint8_t)(60 / (get_num_slots() + 1)),

          /*General config*/ 0x00,
          0x01,
          0x00,

          /*WuSlot*/ temp_display_slot,

          /*GetFirmwareType*/ 0x1F,
          /*GetDisplaySize*/ 0x1A,
        };



        set_buffer_length(sizeof(temp_buffer));
        set_data_to_send(temp_buffer, sizeof(temp_buffer));


        set_is_data_waiting(true);
        set_trans_mode(1);
        set_mode_idle();
      } else {
        wakeup();
      }
    }

    virtual void new_interval()
    {

    }

    virtual void pre()
    {
      log_main(mode_name);
      set_last_activation_status(false);
      last_wu_act = false;
      wakeup_start_time = millis();
      CC1101_set_freq(get_wu_channel());
      CC1101_set_net_id(0);

      memset (tx_wu_buffer, 0xff, 10);
      tx_wu_buffer[0] = 0x00;
      tx_wu_buffer[1] = 0xff;// New Version Activation
      tx_wu_buffer[2] = 0x05;// Num Periods per slot
      tx_wu_buffer[3] = 0x4c;// Slot time in MS LOW
      tx_wu_buffer[4] = 0x04;// Slot time in MS HIGH
      tx_wu_buffer[5] = 0x10;// Num Slots Activation ??
      tx_wu_buffer[6] = 0x02;
      tx_wu_buffer[7] = 0x01;// Frequenzy
      tx_wu_buffer[8] = 0x03;
      tx_wu_buffer[9] = 0xfe;// Used ID for Activation

      Serial.print("Wun Activation: 0x");
      Serial.print(tx_wu_buffer[0], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[1], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[2], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[3], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[4], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[5], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[6], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[7], HEX);
      Serial.print(" 0x");
      Serial.print(tx_wu_buffer[8], HEX);
      Serial.print(" 0x");
      Serial.println(tx_wu_buffer[9], HEX);

      cc1101_prepaire_tx();
      cc1101_idle();
      cc1101_tx();
      set_fifo_interrupt(2, 14);
      wakeup();
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
    String mode_name = "Wakeup Activation";

    long wakeup_start_time;
    uint8_t tx_wu_buffer[10];
    bool last_wu_act = true;

    void wakeup() {
      if (millis() - wakeup_start_time > 15500) {
        log_main("WAKEUP Activation done");
        set_fifo_interrupt(6, 7);
        last_wu_act = true;
        return;
      }
      send_radio_tx_burst(tx_wu_buffer, 10);
    }

};

ModeWunAct modeWunAct;
