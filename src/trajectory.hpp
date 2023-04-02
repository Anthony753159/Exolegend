#pragma once

#include "gladiator.h"
#include "utils.hpp"

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
  bool Goto(const RobotData &data);
  void Update(const RobotData &data);

  bool GotoBaseReached() const;
  bool ShouldSearchStrategy() const;
  bool ShouldApplyStrategy() const;

  TrajectoryMsg::State GetState() const;

private:
  Gladiator *m_gladiator;

  TrajectoryMsg::State m_state;

  float m_goto_x;
  float m_goto_y;
  float m_goto_angle;
  bool m_goto_reverse;
  bool m_goto_base_reached;
  bool m_goto_high_speed;

  float m_start_x;
  float m_start_y;
  float m_distance_remaining;
  bool m_distance_forward;

  unsigned long m_last_time_updated;
  float m_rotate_time_remaining;
  bool m_rotate_direction;
};