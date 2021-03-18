#pragma once
#include "RFV3.h"

class ModeWuAct : public mode_class
{
  public:
    virtual void interrupt()
    {
      if (last_wu_act)
        set_mode_activation();
      else
        wakeup();
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
      tx_wu_buffer[7] = get_freq() + 1;
      tx_wu_buffer[8] = 0x00;
      tx_wu_buffer[9] = get_network_id();

      uint8_t serial[6];
      get_serial(serial);
      memcpy(&tx_wu_buffer[1], serial, 6);

      Serial.print("Wu Activation: 0x");
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

ModeWuAct modeWuAct;
