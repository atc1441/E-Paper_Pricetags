#include <Arduino.h>
#include "interval_timer.h"
#include "main_variables.h"
#include "logger.h"

/*Timer Stuff*/
hw_timer_t *timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile long _last_interval_millis, _increasement;
long last_interval_millis, increasement;

void IRAM_ATTR interval_timer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  _last_interval_millis = millis();
  _increasement++;
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
    last_interval_millis = _last_interval_millis;
    increasement = _increasement;
    _increasement = 0;
    portEXIT_CRITICAL(&timerMux);
    while (increasement--)
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