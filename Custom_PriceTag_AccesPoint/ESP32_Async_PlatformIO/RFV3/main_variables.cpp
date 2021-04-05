#include <Arduino.h>
#include <SPI.h>
#include "main_variables.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "RFV3.h"
#include "interval_timer.h"

/*Base variables*/
volatile int main_state = 0;
volatile uint8_t slot_address = 0x00;
bool last_to_short = false;
int rx_timeout_ms = 40;
/*END Base variables*/

/*Display specific variables*/
bool is_data_waiting = false;
uint8_t data_slot = 0x00;
uint16_t display_id = 0x0000; //0x0000-0xfffe 0xffff = broadcast
uint8_t network_id = 1;       //0-255
/*END Display specific variables*/

/*Activation specific variables*/

uint8_t serial_id[6];
/*END Activation specific variables*/

/*Network variables*/
uint8_t num_slot = 4 - 1; //up to 16, when increased less power is used but it takes longer to update
uint8_t main_freq = 0;
int rounds_to_resync = 500;
/*END Network variables*/

int after_full_sync_count = 5;

void set_num_slot(uint8_t slots)
{
  num_slot = slots - 1;
}

uint8_t wu_channel = 4;
void set_wu_channel(uint8_t channel)
{
  wu_channel = channel;
}

uint8_t get_wu_channel()
{
  return wu_channel;
}

int get_main_state()
{
  return main_state;
}

void set_main_state(int state)
{
  main_state = state;
}

uint8_t get_slot_address()
{
  return slot_address;
}

void increment_slot_address()
{
  if (slot_address >= num_slot)
  {
    slot_address = 0x00;
    after_full_sync_count++;
  }
  else
    slot_address++;
}

uint8_t get_freq()
{
  return main_freq;
}

void set_freq(uint8_t state)
{
  main_freq = state;
}

bool get_is_data_waiting()
{
  if (display_id == 0xffff || (slot_address == data_slot && is_data_waiting && (after_full_sync_count >= 1)))
  {
    return true;
  }
  return false;
}

bool get_is_data_waiting_raw()
{
  return is_data_waiting;
}

void set_is_data_waiting(uint16_t id)
{
  display_id = id;
  data_slot = num_slot & id;
  if(id){
  set_last_send_status(1);
  is_data_waiting = true;
  }else{
  set_last_send_status(0);
  is_data_waiting = false;
  }
}

void reset_full_sync_count()
{
  after_full_sync_count = 0;
}

void set_data_slot(int slot)
{
  data_slot = slot;
}

uint16_t get_display_id()
{
  return display_id;
}

void set_display_id(uint16_t id)
{
  display_id = id;
}

uint8_t get_network_id()
{
  return network_id;
}

void set_network_id(uint8_t state)
{
  network_id = state;
}

uint8_t get_num_slots()
{
  return num_slot;
}

void get_serial(uint8_t serial[])
{
  memcpy(serial, serial_id, 6);
}

void set_serial(uint8_t serial[])
{
  memcpy(serial_id, serial, 6);
}

int get_rounds_to_resync()
{
  return rounds_to_resync;
}

bool get_last_to_short()
{
  return last_to_short;
}

void set_last_to_short(bool state)
{
  last_to_short = state;
}

int get_rx_timeout()
{
  return rx_timeout_ms;
}

int last_activation_status = 0;
int get_last_activation_status()
{
  return last_activation_status;
}

void set_last_activation_status(int state)
{
  last_activation_status = state;
}

int last_send_status = 0;
int get_last_send_status()
{
  return last_send_status;
}

void set_last_send_status(int state)
{
  last_send_status = state;
}

/* New Activation mode */
int trans_mode = 0;
void set_trans_mode(int state)
{
  trans_mode = state;
}

int get_trans_mode()
{
  return trans_mode;
}

uint8_t temp_freq;
uint8_t temp_network_id;
uint8_t temp_num_slots;
void save_current_settings()
{
  temp_freq = get_freq();
  temp_network_id = get_network_id();
  temp_num_slots = get_num_slots() + 1;
}

void restore_current_settings()
{
  set_freq(temp_freq);
  set_network_id(temp_network_id);
  set_num_slot(temp_num_slots);
}
/* END New Activation mode */
