#pragma once

uint8_t get_num_slots();
void set_num_slot(uint8_t slots);
void set_wu_channel(uint8_t channel);
uint8_t get_wu_channel();
int get_main_state();
void set_main_state(int state);
uint8_t get_slot_address();
void increment_slot_address();
uint8_t get_freq();
void set_freq(uint8_t state);
int get_full_sync_max();
bool get_is_data_waiting();
bool get_is_data_waiting_raw();
void set_is_data_waiting(uint16_t id);
void reset_full_sync_count();
void set_data_slot(int slot);
uint16_t get_display_id();
void set_display_id(uint16_t id);
uint8_t get_network_id();
void set_network_id(uint8_t state);
void get_serial(uint8_t serial[]);
void set_serial(uint8_t serial[]);
int get_rounds_to_resync();
bool get_last_to_short();
void set_last_to_short(bool state);
int get_rx_timeout();

int get_last_activation_status();
void set_last_activation_status(int state);
int get_last_send_status();
void set_last_send_status(int state);

/* NEW MODE */
void set_trans_mode(int state);
int get_trans_mode();
void save_current_settings();
void restore_current_settings();