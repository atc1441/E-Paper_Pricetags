#pragma once
#include "RFV3.h"

class ModeActivation : public mode_class
{
public:
  virtual void interrupt()
  {
    if (is_in_tx)
    {
      is_in_tx = false;
      cc1101_rx(0);
      rx_start_time = millis();
    }
    else
    {
      is_in_tx = true;
      read_data_cc1101();
      if (activation_position >= 17)
      {
        cc1101_idle();
        CC1101_set_freq(get_freq());
        CC1101_set_net_id(get_network_id());
        set_last_activation_status(3);
        reset_full_sync_count();
        set_mode_idle();
      }
      else
      {
        activation_position++;
        packet_counter++;
        activation_handler();
      }
    }
  }

  virtual void new_interval()
  {
  }

  virtual void pre()
  {
    log(mode_name);
    activation_position = 0;
    is_in_tx = true;
    packet_counter = 0x80;
    rx_timeout_counter = 0;
    activation_handler();
  }

  virtual void main()
  {
    if (!is_in_tx)
    {
      if (millis() - rx_start_time >= get_rx_timeout())
      {
        if (rx_timeout_counter >= 10)
        {
          log("no connection possible, exit activation");
          cc1101_idle();
          set_mode_idle();
          set_last_activation_status(2);
          return;
        }
        log("RX Timeout retry");
        rx_timeout_counter++;
        activation_handler();
      }
    }
  }

  virtual void post()
  {
  }

  virtual String get_name()
  {
    return mode_name;
  }

private:
  String mode_name = "Activation";
  volatile int activation_position = 0;
  bool is_in_tx = true;
  long rx_start_time = 0;
  uint8_t packet_counter = 0x80;
  int rx_timeout_counter = 0;

  uint8_t tx_act_buffer[16];

  void activation_handler()
  {
    memset(tx_act_buffer, 0x00, 16);

    uint8_t serial[6];
    get_serial(serial);
    memcpy(&tx_act_buffer[1], serial, 6);
    tx_act_buffer[7] = packet_counter;
    if (packet_counter >= 0x8E)
      packet_counter = 0x80;

    switch (activation_position)
    {
    case 0: //Set network id and display id
      tx_act_buffer[8] = 3;
      tx_act_buffer[9] = get_network_id();
      tx_act_buffer[10] = get_display_id() >> 8;
      tx_act_buffer[11] = get_display_id() & 0xff;
      break;
    case 1: //Channel 0
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 0;
      tx_act_buffer[10] = get_freq() + 1;
      break;
    case 2: //Channel 1
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 1;
      tx_act_buffer[10] = 0;
      break;
    case 3: //Channel 2
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 2;
      tx_act_buffer[10] = 0;
      break;
    case 4: //Channel 3
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 3;
      tx_act_buffer[10] = 0;
      break;
    case 5: //Channel 4
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 4;
      tx_act_buffer[10] = 0;
      break;
    case 6: //Channel 5
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 5;
      tx_act_buffer[10] = 0;
      break;
    case 7: //Channel 6
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 6;
      tx_act_buffer[10] = 0;
      break;
    case 8: //Channel 7
      tx_act_buffer[8] = 4;
      tx_act_buffer[9] = 7;
      tx_act_buffer[10] = 0;
      break;
    case 9: //Number of slots
      tx_act_buffer[8] = 5;
      tx_act_buffer[9] = get_num_slots() + 1;
      break;
    case 10: //Time per slot
      tx_act_buffer[8] = 6;
      tx_act_buffer[9] = 0x04;
      tx_act_buffer[10] = 0x4C;
      break;
    case 11: //syncs in short sync mode
      tx_act_buffer[8] = 7;
      tx_act_buffer[9] = 5;
      break;
    case 12: //Length of Message from AP to display
      tx_act_buffer[8] = 8;
      tx_act_buffer[9] = 0x39;
      break;
    case 13: //RF Region 0x40 = EU, 0x38 = US
      tx_act_buffer[8] = 9;
      tx_act_buffer[9] = 0x40;
      break;
    case 14: //Extended Sleep 1092 minutes = 65520 seconds = 0xFFF0
      tx_act_buffer[8] = 10;
      tx_act_buffer[9] = 0xFF;
      tx_act_buffer[10] = 0xF0;
      break;
    case 15: //Max Missed Sync Periods
      tx_act_buffer[8] = 11;
      tx_act_buffer[9] = 0x00;
      tx_act_buffer[10] = 60 / (get_num_slots() + 1);
      break;
    case 16: //General config
      tx_act_buffer[8] = 12;
      tx_act_buffer[9] = 0x00;
      tx_act_buffer[10] = 0x01;
      break;
    case 17:
      tx_act_buffer[8] = 0xff;
      break;
    }

    printf("Activation:");
    for (int i = 0; i < 15; i++)
    {
      printf(" 0x%02x", tx_act_buffer[i]);
    }
    printf("\r\n");

    cc1101_tx_fill(tx_act_buffer, 16);
    cc1101_tx();
    is_in_tx = true;
  }

  bool read_data_cc1101()
  {
    uint8_t data_array[255];
    int read_len = cc1101_read_fifo(data_array);

    if (read_len == -1)
    {
      Serial.println("Error while reading RX buffer");
      spi_write_strobe(CC1101_CMD_SIDLE);
      return false;
    }

    printf("Read_len: %d Data:",read_len);
    for (int i = 0; i < read_len; i++)
    {
      printf(" 0x%02x", data_array[i]);
    }
    printf("\r\n");

    spi_write_strobe(CC1101_CMD_SIDLE);
    return true;
  }
};

ModeActivation modeActivation;
