#pragma once

#include "gladiator.h"

class Strategy
{
public:
  Strategy(Gladiator *gladiator);
  ~Strategy();

  void Update();

private:
  Gladiator *m_gladiator;
};
