#include "strategy.hpp"

#include <math.h>

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator)
{
}

Strategy::~Strategy()
{
}

void Strategy::Update(const RobotData &data, const RobotData *others_data, const RobotList &list)
{
  if (!m_maze_initialized)
  {
    InitMaze();
  }
  UpdateMaze();

  m_state.rewards_we_got = 0;
  m_state.SetTime((millis() - m_match_start_time) * 0.001f);
  size_t my_index = 0;

  for (size_t i = 0; i < N_ROBOTS; i++)
  {
    int8_t ix = (int8_t)(others_data[i].position.x / m_square_size);
    int8_t iy = (int8_t)(others_data[i].position.y / m_square_size);

    if (others_data[i].id == 0)
    {
      ix = 13 * (i % 2);
      iy = 13 * (i >= 2 ? 1 : 0);
    }

    int8_t idir = 1; /* East */
    if (others_data[i].position.a > 1 * M_PI / 4 && others_data[i].position.a <= 3 * M_PI / 4)
    {
      idir = 0; /* North */
    }
    else if (others_data[i].position.a > 3 * M_PI / 4 && others_data[i].position.a <= 5 * M_PI / 4)
    {
      idir = 3; /* West */
    }
    else if (others_data[i].position.a > 5 * M_PI / 4 && others_data[i].position.a <= 7 * M_PI / 4)
    {
      idir = 2; /* South */
    }

    m_state.robots[i].pos = {ix, iy};
    m_state.robots[i].direction = idir;

    m_state.robots[i].next_action_time = m_state.time;
    m_state.robots[i].ally = data.teamId == others_data[i].teamId;
    m_state.robots[i].remaining_slow_down = 0.0f;

    if (others_data[i].id == data.id)
    {
      my_index = i;
    }
  }

  Action action = MonteCarloTreeSearch(m_state, m_gladiator, my_index);
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
    m_next_msg.goto_x = m_state.robots[my_index].GetNorth().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.robots[my_index].GetNorth().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = M_PI / 2;
    move_dir = 0;
    break;

  case Action::MOVE_EAST:
    m_gladiator->log("MOVE_EAST");
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.robots[my_index].GetEast().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.robots[my_index].GetEast().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = 0;
    move_dir = 1;
    break;

  case Action::MOVE_SOUTH:
    m_gladiator->log("MOVE_SOUTH");
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.robots[my_index].GetSouth().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.robots[my_index].GetSouth().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = -M_PI / 2;
    move_dir = 2;
    break;

  case Action::MOVE_WEST:
    m_gladiator->log("MOVE_WEST");
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.robots[my_index].GetWest().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.robots[my_index].GetWest().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = M_PI;
    move_dir = 3;
    break;

  case Action::UNDEFINED:
    break;
  }

  if (move_dir != -1)
  {
    if (MazeWalls::GetInstance()->IsWall(m_state.robots[my_index].pos.x, m_state.robots[my_index].pos.y, move_dir))
    {
      m_state.robots[my_index].wall_hits++;
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
