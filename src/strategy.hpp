#pragma once

#include "gladiator.h"
#include "comms.hpp"
#include "algos/common.hpp"

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

  /* Maze */
  bool m_maze_initialized = false;
  float m_maze_size;
  float m_square_size;

  float m_rewards[MAZE_SIZE * MAZE_SIZE] = {0};
};
