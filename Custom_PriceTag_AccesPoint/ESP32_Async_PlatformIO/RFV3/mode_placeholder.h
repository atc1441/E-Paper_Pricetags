#pragma once
#include "RFV3.h"

class ModePlaceholder : public mode_class
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
    log(mode_name);
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
  String mode_name = "Placeholder";
};

ModePlaceholder modePlaceholder;
