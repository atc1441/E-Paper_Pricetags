#pragma once
#include "Arduino.h"

// This file simply creates the class the different modes can use

class mode_class
{
  public:
    virtual void interrupt()
    {
    }

    virtual void new_interval()
    {
    }

    virtual void pre()
    {
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
    String mode_name = "Not set";
};
