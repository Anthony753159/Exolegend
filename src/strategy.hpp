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

  void Update(const RobotData &data);
  TrajectoryMsg GetNextMsg() const;

  bool IsNextMsgValid() const;
  void ConsumeMsg();

private:
  void InitMaze();
  void UpdateMaze();

  Gladiator *m_gladiator;

  bool m_maze_initialized = false;
  float m_maze_size;
  float m_square_size;

  GameState m_state;
  unsigned long m_match_start_time;

  TrajectoryMsg m_next_msg;
  bool m_next_msg_valid = false;
  bool m_previous_goto_reverse = true;
  Action m_previous_action = Action::UNDEFINED;
};
