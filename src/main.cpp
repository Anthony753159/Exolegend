#include "gladiator.h"

#include "strategy.hpp"
#include "trajectory.hpp"

#define FREE_MODE false

Gladiator *gladiator;
Strategy *strategy = nullptr;
Trajectory *trajectory = nullptr;

void reset();

void setup()
{
  gladiator = new Gladiator();
  strategy = new Strategy(gladiator);
  trajectory = new Trajectory(gladiator);

  if (FREE_MODE)
  {
    gladiator->game->enableFreeMode(RemoteMode::OFF);
  }

  gladiator->game->onReset(&reset);
}

void reset()
{
  if (strategy != nullptr)
  {
    delete strategy;
  }
  strategy = new Strategy(gladiator);

  if (trajectory != nullptr)
  {
    delete trajectory;
  }
  trajectory = new Trajectory(gladiator);
}

RobotData robot_data;

void loop()
{
  if (gladiator->game->isStarted())
  {
    robot_data = gladiator->robot->getData();

    trajectory->Update(robot_data);

    if (trajectory->GetState() == TrajectoryMsg::State::IDLE)
    {
      TrajectoryMsg msg = strategy->Update(robot_data);
      if (msg.order != TrajectoryMsg::UNDEFINED)
      {
        trajectory->HandleMessage(msg);
      }
    }

    delay(10);
  }
}