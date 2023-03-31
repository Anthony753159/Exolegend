

#include "strategy.hpp"

#define WHEEL_TURN_SPEED 0.2
#define WHEEL_FORWARD_SPEED 0.6

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator)
{
  m_state_index = 0;
  m_state = m_state_sequence[m_state_index];
}

Strategy::~Strategy()
{
}

void Strategy::Update()
{
  switch (m_state)
  {
  case IDLE:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
    break;

  case FORWARD:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_FORWARD_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_FORWARD_SPEED);
    break;

  case BACKWARD:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_FORWARD_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -WHEEL_FORWARD_SPEED);
    break;

  case LEFT:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_TURN_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -WHEEL_TURN_SPEED);
    break;

  case RIGHT:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_TURN_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_TURN_SPEED);
    break;
  }

  UpdateState();

  delay(100);
  m_updates++;
}

void Strategy::UpdateState()
{
  if (m_updates > 20)
  {
    m_updates = 0;
    m_state_index = (m_state_index + 1) % 8;
    m_state = m_state_sequence[m_state_index];
  }
}