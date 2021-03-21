#include <Arduino.h>
#include "interval_timer.h"
#include "main_variables.h"
#include "logger.h"

/*Timer Stuff*/
hw_timer_t *timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile long _last_interval_millis, _interval_counter;
long last_interval_millis, interval_counter;

void IRAM_ATTR interval_timer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  _interval_counter++;
  _last_interval_millis = millis();
  portEXIT_CRITICAL_ISR(&timerMux);
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void init_timer()
{
  log_verbose("Interval timer init");
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &interval_timer, true);
  timerAlarmWrite(timer, 1100000, true);
  timerAlarmEnable(timer);
  log_verbose("Interval timer done");
}

bool check_new_interval()
{
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
  {
    portENTER_CRITICAL(&timerMux);
    interval_counter = _interval_counter;
    last_interval_millis = _last_interval_millis;
    portEXIT_CRITICAL(&timerMux);
    increment_slot_address();
    return 1;
  }
  return 0;
}

long get_last_slot_time()
{
  return last_interval_millis;
}
/*END Timer Stuff*/