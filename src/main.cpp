#include "gladiator.h"

#include "strategy.hpp"
#include "trajectory.hpp"
#include "algos/common.hpp"

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

  MazeWalls::ResetInstance();
}

RobotData robot_data;
RobotData other_robots_data[N_ROBOTS];
RobotList robot_list;
bool once = true;

void loop()
{
  if (gladiator->game->isStarted())
  {
    if (once)
    {
      once = false;
      robot_list = gladiator->game->getPlayingRobotsId();
    }

    robot_data = gladiator->robot->getData();
    for (size_t i = 0; i < N_ROBOTS; i++)
    {
      other_robots_data[i] = gladiator->game->getOtherRobotData(robot_list.ids[i]);
    }

    trajectory->Update(robot_data);

    if (trajectory->ShouldSearchStrategy())
    {
      if (!strategy->IsNextMsgValid())
      {
        strategy->Update(robot_data, robot_list, other_robots_data);
      }
    }

    if (trajectory->ShouldApplyStrategy())
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