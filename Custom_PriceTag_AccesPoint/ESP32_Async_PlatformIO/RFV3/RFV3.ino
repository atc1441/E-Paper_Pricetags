#include <Arduino.h>
#include <SPI.h>
#include "RFV3.h"
#include "main_variables.h"
#include "logger.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "class.h"
#include "compression.h"
#include "interval_timer.h"
#include "web.h"
#include "utils.h"
#include "mode_sync.h"
#include "mode_full_sync.h"
#include "mode_idle.h"
#include "mode_trans.h"
#include "mode_wu.h"
#include "mode_wu_activation.h"
#include "mode_wun_activation.h"
#include "mode_activation.h"
#include "mode_placeholder.h"

mode_class *currentMode = &modePlaceholder;
mode_class *tempMode = &modeIdle;

volatile int interrupt_counter = 0;

volatile bool int_fired = false;
void IRAM_ATTR GDO2_interrupt()
{
  interrupt_counter++;
    int_fired = true;
}

void init_interrupt()
{
  log_verbose("GDO2 interrupt init");
  pinMode(GDO2, INPUT);
  attachInterrupt(GDO2, GDO2_interrupt, FALLING);
  log_verbose("GDO2 interrupt init done");
}

void setup()
{
  init_log();
  init_spi();
  if (!init_radio())
  {
    while (1)
    {
      log_verbose("Radio not working!!! ERROR");
      delay(1000);
    }
  }
  init_interrupt();
  init_web();
}

void loop()
{
  if (int_fired)
  {
    int_fired = false;
    currentMode->interrupt();
  }
  if (currentMode != tempMode)
  {
    currentMode->post();
    currentMode = tempMode;
    log_main("Mode changed to " + currentMode->get_name());
    currentMode->pre();
  }
  currentMode->main();
  if (check_new_interval())
  {
    log_main("Count: "+ String(interrupt_counter));
    currentMode->new_interval();
  }
}

void set_mode_idle()
{
  tempMode = &modeIdle;
}

void set_mode_sync()
{
  tempMode = &modeSync;
}

void set_mode_full_sync()
{
  tempMode = &modeFullSync;
}

void set_mode_trans()
{
  tempMode = &modeTrans;
}

void set_mode_wu()
{
  tempMode = &modeWu;
}

void set_mode_wu_activation()
{
  tempMode = &modeWuAct;
}

void set_mode_wun_activation()
{
  tempMode = &modeWunAct;
}

void set_mode_activation()
{
  tempMode = &modeActivation;
}

String get_mode_string()
{
  return currentMode->get_name();
}
