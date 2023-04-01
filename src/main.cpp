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
RobotList robot_list;
RobotData other_robots_data[N_ROBOTS];

void loop()
{
  if (gladiator->game->isStarted())
  {
    robot_data = gladiator->robot->getData();
    robot_list = gladiator->game->getPlayingRobotsId();
    for (size_t i = 0; i < N_ROBOTS; i++)
    {
      other_robots_data[i] = gladiator->game->getOtherRobotData(robot_list.ids[i]);
    }

    trajectory->Update(robot_data);

    if (trajectory->GotoBaseReached() || trajectory->GetState() == TrajectoryMsg::State::IDLE)
    {
      if (!strategy->IsNextMsgValid())
      {
        strategy->Update(robot_data, other_robots_data, robot_list);
      }
    }

    if (trajectory->GetState() == TrajectoryMsg::State::IDLE)
    {
      TrajectoryMsg msg = strategy->GetNextMsg();
      strategy->ConsumeMsg();
      if (msg.order != TrajectoryMsg::UNDEFINED)
      {
        trajectory->HandleMessage(msg);
      }
    }
  }
}