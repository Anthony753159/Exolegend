#include "strategy.hpp"

#include <math.h>

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator)
{
}

Strategy::~Strategy()
{
}

void Strategy::Update(const RobotData &data, const RobotList &list, RobotData *others_data)
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

  m_state.pos = {ix, iy};
  m_state.visits[ix + iy * MAZE_SIZE]++;
  m_state.direction = idir;
  m_state.remaining_slow_down = 0.0f;
  m_state.rewards_we_got = 0;
  m_state.SetTime((millis() - m_match_start_time) * 0.001f);

  bool enemy_in_range = false;
  size_t attackable_enemy_id = N_ROBOTS;

  GameState altered_state{m_state};
  for (size_t i = 0; i < N_ROBOTS; i++)
  {
    if (others_data[i].id == data.id)
    {
      continue;
    }

    int8_t oix = (int8_t)(others_data[i].position.x / m_square_size);
    int8_t oiy = (int8_t)(others_data[i].position.y / m_square_size);

    for (int8_t x = std::max(0, oix - 1); x <= std::min(MAZE_SIZE - 1, oix + 1); x++)
    {
      for (int8_t y = std::max(0, oiy - 1); y <= std::min(MAZE_SIZE - 1, oiy + 1); y++)
      {
        altered_state.rewards[x + y * MAZE_SIZE] = 0.1f * altered_state.rewards[x + y * MAZE_SIZE];
      }
    }

    if (others_data[i].teamId != data.teamId)
    {
      if (others_data[i].lifes == 0)
      {
        continue;
      }

      float cells_distance = sqrtf((oix - ix) * (oix - ix) + (oiy - iy) * (oiy - iy));
      if (cells_distance <= ENEMY_DETECTION_RANGE)
      {
        enemy_in_range = true;

        /* Is the enemy turning its back to us and is in a straight line from us */
        float angle_from_me_to_enemy = atan2f(others_data[i].position.y - data.position.y, others_data[i].position.x - data.position.x);
        float angle_enemy_facing = others_data[i].position.a;
        float angle_diff = AngleDiffRad(angle_from_me_to_enemy, angle_enemy_facing);

        bool enemy_facing_away = Abs(angle_diff) < M_PI / 2;
        bool enemy_in_straigh_line = ix == oix || iy == oiy;

        if (enemy_facing_away && enemy_in_straigh_line)
        {
          attackable_enemy_id = i;
        }
      }
    }
  }

  Action action = MonteCarloTreeSearch(altered_state, m_gladiator);

  int8_t move_dir = -1;

  if (m_previous_action == Action::MOVE_NORTH && action == Action::MOVE_SOUTH)
  {
    m_next_msg.goto_reverse = !m_previous_goto_reverse;
  }
  else if (m_previous_action == Action::MOVE_SOUTH && action == Action::MOVE_NORTH)
  {
    m_next_msg.goto_reverse = !m_previous_goto_reverse;
  }
  else if (m_previous_action == Action::MOVE_EAST && action == Action::MOVE_WEST)
  {
    m_next_msg.goto_reverse = !m_previous_goto_reverse;
  }
  else if (m_previous_action == Action::MOVE_WEST && action == Action::MOVE_EAST)
  {
    m_next_msg.goto_reverse = !m_previous_goto_reverse;
  }
  else if (m_previous_action == action)
  {
    m_next_msg.goto_reverse = m_previous_goto_reverse;
  }
  else
  {
    m_next_msg.goto_reverse = true;
  }

  m_previous_action = action;
  m_previous_goto_reverse = m_next_msg.goto_reverse;

  if (enemy_in_range)
  {
    if (attackable_enemy_id != N_ROBOTS)
    {
      action = Action::ATTACK;
    }
    else
    {
      action = Action::DEFEND;
    }
  }

  m_next_msg.goto_high_speed = false;

  switch (action)
  {
  case Action::MOVE_NORTH:
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetNorth().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetNorth().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = M_PI / 2;
    move_dir = 0;
    break;

  case Action::MOVE_EAST:
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetEast().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetEast().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = 0;
    move_dir = 1;
    break;

  case Action::MOVE_SOUTH:
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetSouth().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetSouth().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = -M_PI / 2;
    move_dir = 2;
    break;

  case Action::MOVE_WEST:
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = m_state.GetWest().x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_y = m_state.GetWest().y * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_angle = M_PI;
    move_dir = 3;
    break;

  case Action::ATTACK:
    m_next_msg.order = TrajectoryMsg::ORDER_GOTO;
    m_next_msg.goto_x = others_data[attackable_enemy_id].position.x;
    m_next_msg.goto_y = others_data[attackable_enemy_id].position.y;
    m_next_msg.goto_angle = atan2f(m_next_msg.goto_y - data.position.y, m_next_msg.goto_x - data.position.x);
    m_next_msg.goto_high_speed = true;
    m_next_msg.goto_reverse = false;
    break;

  case Action::DEFEND:
    m_next_msg.order = TrajectoryMsg::ORDER_ROTATE;
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
  MazeWalls::GetInstance()->gladiator = m_gladiator;

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
