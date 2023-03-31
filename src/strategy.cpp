

#include "strategy.hpp"

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator)
{
}

Strategy::~Strategy()
{
}

void Strategy::Update()
{
  m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0.6);
  m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0.6);
  delay(2000);

  m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -0.6);
  m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -0.6);
  delay(2000);
}