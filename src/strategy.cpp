

#include "strategy.hpp"

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator)
{
  m_state_index = 0;
}

Strategy::~Strategy()
{
}

TrajectoryMsg Strategy::Update(const RobotData &data)
{
  TrajectoryMsg msg;

  m_updates++;

  if (m_updates > 10)
  {
    m_updates = 0;
    m_state_index = (m_state_index + 1) % 8;

    msg.order = TrajectoryMsg::SET_STATE;
    msg.state = m_state_sequence[m_state_index];
  }

  return msg;
}
