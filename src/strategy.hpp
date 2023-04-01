#pragma once

#include "gladiator.h"
#include "comms.hpp"
#include "algos/common.hpp"
#include "algos/astar.hpp"
#include "algos/montecarlo.hpp"

class Strategy
{
public:
  Strategy(Gladiator *gladiator);
  ~Strategy();

  TrajectoryMsg Update(const RobotData &data);

private:
  void InitMaze();
  void UpdateMaze();

  Gladiator *m_gladiator;

  bool m_maze_initialized = false;
  float m_maze_size;
  float m_square_size;

  GameState m_state;
  unsigned long m_match_start_time;
};
