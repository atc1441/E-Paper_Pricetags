#pragma once
#include "RFV3.h"

class ModeWuAct : public mode_class
{
public:
  virtual void interrupt()
  {
    if (handle_wu())
    {
      log(mode_name + " done");
      set_mode_activation();
    }
  }

  virtual void new_interval()
  {
    check_wu_timeout();
  }

  virtual void pre()
  {
    log(mode_name);
    set_last_activation_status(1);
    start_wu(WU_ACTIVATION_CMD);
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
};

ModeWuAct modeWuAct;
