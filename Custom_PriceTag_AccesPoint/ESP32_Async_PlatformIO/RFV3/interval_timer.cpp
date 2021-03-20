#include <Arduino.h>
#include "interval_timer.h"
#include "main_variables.h"
#include "logger.h"

long last_interval_millis, next_slot_time;

bool check_new_interval() {
  if (millis() >= next_slot_time)
  {
    next_slot_time = millis() + 1100;
    last_interval_millis = millis();
    log_normal("New interval reached");    
    increment_slot_address();
    return 1;
  }
  return 0;
}

long get_last_slot_time(){
  return last_interval_millis;
}
