#include "strategy.hpp"

#include <math.h>

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator)
{
}

Strategy::~Strategy()
{
}

void Strategy::Update(const RobotData &data)
{
  if (!m_maze_initialized)
  {
    InitMaze();
  }
  UpdateMaze();

  int8_t ix = (int8_t)(data.position.x / m_square_size);
  int8_t iy = (int8_t)(data.position.y / m_square_size);

  int8_t idir = 1; /* East */
  if (data.position.a > 1 * M_PI / 4 && data.position.a <= 3 * M_PI / 4)
  {
    idir = 0; /* North */
  }
  else if (data.position.a > 3 * M_PI / 4 && data.position.a <= 5 * M_PI / 4)
  {
    idir = 3; /* West */
  }
  else if (data.position.a > 5 * M_PI / 4 && data.position.a <= 7 * M_PI / 4)
  {
    idir = 2; /* South */
  }

  m_gladiator->log("Robot position: (%d, %d, %d), angle: %f", ix, iy, idir, data.position.a);

  m_state.pos = {ix, iy};
  m_state.direction = idir;
  m_state.remaining_slow_down = 0.0f;
  m_state.rewards_we_got = 0;
  m_state.SetTime((millis() - m_match_start_time) * 0.001f);

  m_gladiator->log("Time: %f, Maze retract: %d, %lu", m_state.time, m_state.maze_retract, millis());

  Action action = MonteCarloTreeSearch(m_state, m_gladiator);
  int8_t move_dir = -1;

  m_next_msg.goto_reverse = m_previous_goto_reverse;
  if (action != m_previous_action)
  {
    m_next_msg.goto_reverse = !m_next_msg.goto_reverse;
  }
  m_previous_action = action;
  m_previous_goto_reverse = m_next_msg.goto_reverse;

  switch (action)
  {
  case Action::MOVE_NORTH:
    m_gladiator->log("MOVE_NORTH");
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetNorth().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetNorth().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = M_PI / 2;
    move_dir = 0;
    break;

  case Action::MOVE_EAST:
    m_gladiator->log("MOVE_EAST");
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetEast().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetEast().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = 0;
    move_dir = 1;
    break;

  case Action::MOVE_SOUTH:
    m_gladiator->log("MOVE_SOUTH");
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetSouth().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetSouth().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = -M_PI / 2;
    move_dir = 2;
    break;

  case Action::MOVE_WEST:
    m_gladiator->log("MOVE_WEST");
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetWest().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetWest().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = M_PI;
    move_dir = 3;
    break;

  case Action::UNDEFINED:
    break;
  }

  if (move_dir != -1)
  {
    if (MazeWalls::GetInstance()->IsWall(ix, iy, move_dir))
    {
      m_state.wall_hits++;
    }
  }

  m_next_msg_valid = true;
}

TrajectoryMsg Strategy::GetNextMsg() const
{
  return m_next_msg;
}

bool Strategy::IsNextMsgValid() const
{
  return m_next_msg_valid;
}

void Strategy::ConsumeMsg()
{
  m_next_msg_valid = false;
}

void Strategy::InitMaze()
{
  m_match_start_time = millis();

  m_maze_size = m_gladiator->maze->getSize();
  m_square_size = m_gladiator->maze->getSquareSize();

  for (size_t y = 0; y < MAZE_SIZE; y++)
  {
    for (size_t x = 0; x < MAZE_SIZE; x++)
    {
      MazeSquare sqr = m_gladiator->maze->getSquare(x, y);

      m_state.rewards[x + y * MAZE_SIZE] = sqr.coin.value;

      if (x < MAZE_SIZE - 1)
      {
        MazeWalls::GetInstance()->horizontal_walls[x + y * (MAZE_SIZE - 1)] = sqr.eastSquare == nullptr;
      }
      if (y < MAZE_SIZE - 1)
      {
        MazeWalls::GetInstance()->vertical_walls[x + y * MAZE_SIZE] = sqr.northSquare == nullptr;
      }
    }
  }

  m_maze_initialized = true;
}

void Strategy::UpdateMaze()
{
  m_state.sum_of_rewards = 0;

  for (size_t x = 0; x < MAZE_SIZE; x++)
  {
    for (size_t y = 0; y < MAZE_SIZE; y++)
    {
      MazeSquare sqr = m_gladiator->maze->getSquare(x, y);
      m_state.rewards[x + y * MAZE_SIZE] = sqr.coin.value;
      m_state.sum_of_rewards += sqr.coin.value;
    }
  }
}
