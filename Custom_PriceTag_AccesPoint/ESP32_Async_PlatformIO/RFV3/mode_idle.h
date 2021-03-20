#pragma once
#include "RFV3.h"

class ModeIdle : public mode_class
{
  public:
    virtual void interrupt()
    {

    }

    virtual void new_interval()
    {
      resync_start_counter++;
      if (resync_start_counter >= get_rounds_to_resync() && !get_last_to_short()) {
        resync_start_counter = 0;
        set_mode_wu();
      } else {
        set_mode_sync();
      }
    }

    virtual void pre()
    {
      log_main(mode_name);

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
    String mode_name = "Idle";
    int resync_start_counter = 90;

};

ModeIdle modeIdle;
