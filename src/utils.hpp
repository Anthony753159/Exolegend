#pragma once

#include "gladiator.h"
#include "algos/gamestate.hpp"

struct GameData
{
  RobotData robot_data;
  RobotData other_robots_data[N_ROBOTS];
  RobotList robot_list;
  bool first_loop = true;
};

enum TrajectoryState
{
  IDLE,
  ROTATE,
  GOTO
};

struct GotoParameters
{
  float x;
  float y;
  float angle;
  bool backward;
  bool high_speed;
};

struct StrategyToTrajectory
{
  TrajectoryState ordered_state = IDLE;
  GotoParameters goto_parameters;
};

/* Always give the SMALLEST angle from angle 'from' to angle 'to' */
float AngleDiffRad(float from, float to);
