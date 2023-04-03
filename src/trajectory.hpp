#pragma once

#include "gladiator.h"
#include "utils.hpp"

/* Trajectory parameters */

#define RECOMPUTE_BACKWARD true
#define POSITION_SHIFT 0.02f

#define GOTO_BASE_DISTANCE_THRESHOLD 0.06f
#define GOTO_DELTA -0.03f

#define ROTATE_TIME 1.0f

/* Speed parameters */

#define WHEEL_FORWARD_SPEED 0.5f
#define WHEEL_BACKWARD_SPEED 0.35f
#define WHEEL_TURN_SPEED_FORWARD 0.5f
#define WHEEL_TURN_SPEED_BACKWARD 0.5f
#define WHEEL_HIGHSPEED_FACTOR 2.0f

struct Vec2f
{
  float x;
  float y;
};

/* The Trajectory is responsible for making the robot move and reach the objectives decided by the Strategy.
 * It also tells whenever is it safe to start computing the next strategy steps, and when it is available to
 * apply the next step. */
class Trajectory
{
public:
  Trajectory(Gladiator *gladiator);

  void HandleMessage(const StrategyToTrajectory &msg);
  void Update(const GameData &data);

  bool ShouldSearchStrategy() const;
  bool ShouldApplyStrategy() const;

private:
  bool Goto(const RobotData &data);

  Gladiator *m_gladiator;

  TrajectoryState m_state;
  unsigned long m_last_update_ms;

  GotoParameters m_goto_params;
  bool m_goto_base_reached;

  float m_rotate_time_remaining;
  bool m_rotate_direction;
};