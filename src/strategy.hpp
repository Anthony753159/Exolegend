#pragma once

#include "gladiator.h"
#include "utils.hpp"
#include "algos/gamestate.hpp"
#include "algos/montecarlo.hpp"

#define ENEMY_DETECTION_RANGE 2.0f

/* The Strategy is responsible for computing the best course of action, on a high-level. It does not handle
 * the actual movement and only works in a discretized space. It is also responsible for deciding when to
 * attack the enemy, and when to defend. */
class Strategy
{
public:
  Strategy(Gladiator *gladiator);

  void Update(const GameData &data);

  StrategyToTrajectory GetNextMsg() const;
  bool IsNextMsgValid() const;
  void ConsumeMsg();

private:
  void InitMaze();
  void UpdateMaze();

  Gladiator *m_gladiator;

  MazeWalls m_walls;
  bool m_maze_initialized = false;
  float m_maze_size;
  float m_square_size;

  GameState m_state;
  unsigned long m_match_start_ms;

  StrategyToTrajectory m_next_msg;
  bool m_next_msg_valid = false;
  bool m_previous_goto_backward = true;
  Action m_previous_action = Action::UNDEFINED;
};
