#pragma once

#include "gladiator.h"

class Strategy
{
public:
  Strategy(Gladiator *gladiator);
  ~Strategy();

  void Update();
  void UpdateState();

private:
  Gladiator *m_gladiator;

  enum State
  {
    IDLE,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
  } m_state;

  size_t m_state_index;
  State m_state_sequence[8] = {
      State::IDLE,
      State::FORWARD,
      State::LEFT,
      State::FORWARD,
      State::RIGHT,
      State::FORWARD,
      State::BACKWARD,
      State::IDLE,
  };

  size_t m_updates = 0;
};
