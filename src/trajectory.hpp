#pragma once

#include "gladiator.h"
#include "comms.hpp"

class Trajectory
{
public:
  Trajectory(Gladiator *gladiator);
  ~Trajectory();

  void HandleMessage(const TrajectoryMsg &msg);
  void Update(const RobotData &data);

private:
  Gladiator *m_gladiator;

  TrajectoryMsg::State m_state;
};