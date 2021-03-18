#pragma once


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
void set_is_data_waiting(bool state);
void set_data_slot(int slot);
uint16_t get_display_id();
void set_display_id(uint16_t id);
uint8_t get_network_id();
void set_network_id(uint8_t state);
uint8_t get_num_slots();
void get_serial(uint8_t serial[]);
void set_serial(uint8_t serial[]);
int get_rounds_to_resync();
bool get_last_to_short();
void set_last_to_short(bool state);
int get_rx_timeout();

bool get_last_activation_status();
void set_last_activation_status(bool state);


void set_buffer_length(int length);
int get_buffer_length();


void set_buffer_length_answer(int length);
int get_buffer_length_answer();
void set_data_to_send(uint8_t *buffer_in, int length);
void copy_buffer(uint8_t *buffer_out, int length);
void copy_buffer_answer(uint8_t *buffer_out, int length);
void get_data_to_send(uint8_t *buffer_out, int length);



void set_sniff_freq(uint8_t state);
void set_sniff_net_id(uint8_t state);
uint8_t get_sniff_freq();
uint8_t get_sniff_net_id();
void set_buffer_length_sniff(int length);
int get_buffer_length_sniff();
void copy_buffer_sniff(uint8_t *buffer_out, int length);
void get_data_to_send_sniff(uint8_t *buffer_out, int length);


/* NEW MODE */
void set_trans_mode(int state);
int get_trans_mode();
void save_current_settings();
void restore_current_settings();
