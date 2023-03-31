#pragma once

#include "gladiator.h"
#include "comms.hpp"

class Strategy
{
public:
  Strategy(Gladiator *gladiator);
  ~Strategy();

  TrajectoryMsg Update();

private:
  Gladiator *m_gladiator;

  size_t m_state_index;
  TrajectoryMsg::State m_state_sequence[9] = {
      TrajectoryMsg::State::IDLE,
      TrajectoryMsg::State::FORWARD,
      TrajectoryMsg::State::LEFT,
      TrajectoryMsg::State::IDLE,
      TrajectoryMsg::State::FORWARD,
      TrajectoryMsg::State::RIGHT,
      TrajectoryMsg::State::IDLE,
      TrajectoryMsg::State::BACKWARD,
      TrajectoryMsg::State::FORWARD,
  };

  size_t m_updates = 10000;
};
