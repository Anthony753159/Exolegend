#pragma once

#include "gladiator.h"
#include "comms.hpp"

struct Vec2f
{
  float x;
  float y;
};

class Trajectory
{
public:
  Trajectory(Gladiator *gladiator);
  ~Trajectory();

  void HandleMessage(const TrajectoryMsg &msg);
  bool Goto(const RobotData &data, const Vec2f &target, float speed);
  void Update(const RobotData &data);

  TrajectoryMsg::State GetState() const;

private:
  Gladiator *m_gladiator;

  TrajectoryMsg::State m_state;

  float m_goto_x;
  float m_goto_y;

  float m_target_angle;

  float m_start_x;
  float m_start_y;
  float m_distance_remaining;
  bool m_distance_forward;
};