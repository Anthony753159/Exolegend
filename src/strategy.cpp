#include "strategy.hpp"

#include <math.h>

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator) {}

void Strategy::Update(const GameData &data)
{
  if (!m_maze_initialized)
  {
    InitMaze();
  }
  UpdateMaze();

  const RobotData &robot = data.robot_data;

  /* Discretizing robot data */

  int8_t ix = (int8_t)(robot.position.x / m_square_size);
  int8_t iy = (int8_t)(robot.position.y / m_square_size);

  Direction idir = AngleToDirection(robot.position.a);

  /* Updating current GameState with new data */

  m_state.pos = {ix, iy};
  m_state.visits[ix + iy * MAZE_SIZE]++;
  m_state.direction = idir;
  m_state.remaining_slow_down = 0.0f; // Too complicated to compute
  m_state.rewards_we_got = 0;         // We don't care how much we got, we only care about the future!
  m_state.UpdateTime((millis() - m_match_start_ms) * 0.001f);

  /* Checking the presence of an enemy or the ally
   * NOTE: We did not think of taking the dead corpses into account... it did cost us some baloons :P */

  bool enemy_in_range = false;
  size_t attackable_enemy_id = N_ROBOTS;

  GameState altered_state{m_state};
  RobotData other_robot;
  for (size_t i = 0; i < N_ROBOTS; i++)
  {
    other_robot = data.other_robots_data[i];
    if (other_robot.id == robot.id)
    {
      /* It is us. */
      continue;
    }

    /* Discretizing other robot's position */
    int8_t oix = (int8_t)(other_robot.position.x / m_square_size);
    int8_t oiy = (int8_t)(other_robot.position.y / m_square_size);

    /* Reducing the rewards all around the other robots, allies or enemies, to avoid them
     * It DOES work to avoid the corpses a little while, but at some point it is not enough. */
    for (int8_t x = std::max(0, oix - 1); x <= std::min(MAZE_SIZE - 1, oix + 1); x++)
    {
      for (int8_t y = std::max(0, oiy - 1); y <= std::min(MAZE_SIZE - 1, oiy + 1); y++)
      {
        altered_state.rewards[x + y * MAZE_SIZE] = 0.1f * altered_state.rewards[x + y * MAZE_SIZE];
      }
    }

    if (other_robot.teamId != robot.teamId)
    {
      /* It is an enemy... */
      if (other_robot.lifes == 0)
      {
        /* ...but it is dead. */
        continue;
      }

      float cells_distance = sqrtf((oix - ix) * (oix - ix) + (oiy - iy) * (oiy - iy));
      if (cells_distance <= ENEMY_DETECTION_RANGE)
      {
        /* ... and it is close! */
        enemy_in_range = true;

        /* Is the enemy turning its back to us and is it in a straight line from us */
        float angle_from_me_to_enemy = atan2f(other_robot.position.y - robot.position.y, other_robot.position.x - robot.position.x);
        float angle_enemy_facing = other_robot.position.a;
        float angle_diff = AngleDiffRad(angle_from_me_to_enemy, angle_enemy_facing);

        /* We just avoid a PI/4 sector in front of the enemy */
        bool enemy_facing_away = abs(angle_diff) < 3 * M_PI / 4;
        bool enemy_in_straigh_line = ix == oix || iy == oiy;

        if (enemy_facing_away && enemy_in_straigh_line)
        {
          attackable_enemy_id = i;
        }

        /* NOTE: We thinked about removing the 'straight line' condition, but we figured
         * our collecting behavior was good enough we didn't want to risk losing it by being
         * overly agressive. This way, we statistically only attack when the map gets small. */
      }
    }
  }

  Action action = MonteCarloTreeSearch(altered_state, m_walls);

  /* We figured the robot is better at turning fast when going backwards, so we make sure
   * that whenever a turn is needed, we are going backwards. */
  Direction move_dir = Direction::NORTH;
  if (Opposite(ActionToDirection(m_previous_action), ActionToDirection(action)))
  {
    /* In the case we just go straight the opposite direction, we just invert the moving direction */
    m_next_msg.goto_parameters.backward = !m_previous_goto_backward;
  }
  else if (m_previous_action == action)
  {
    /* If the new action is the same as the previous one, we change nothing */
    m_next_msg.goto_parameters.backward = m_previous_goto_backward;
  }
  else
  {
    /* In all other cases, we are turning, so we do want to go in backward mode */
    m_next_msg.goto_parameters.backward = true;
  }

  m_previous_action = action;
  m_previous_goto_backward = m_next_msg.goto_parameters.backward;

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

  m_next_msg.goto_parameters.high_speed = false;

  /* If action is a movement, setup the GOTO order */
  if (IsMovement(action))
  {
    m_next_msg.ordered_state = TrajectoryState::GOTO;
    move_dir = ActionToDirection(action);
    m_next_msg.goto_parameters.angle = DirectionToAngle(move_dir);
    Vec2i dst = m_state.pos + move_dir;
    m_next_msg.goto_parameters.x = dst.x * m_square_size + 0.5f * m_square_size;
    m_next_msg.goto_parameters.y = dst.y * m_square_size + 0.5f * m_square_size;
  }

  /* In other cases... */
  switch (action)
  {
  case Action::ATTACK:
    /* ... setup a GOTO order with high-speed towards the enemy, in forward mode of course! */
    m_next_msg.ordered_state = TrajectoryState::GOTO;
    m_next_msg.goto_parameters.x = data.other_robots_data[attackable_enemy_id].position.x;
    m_next_msg.goto_parameters.y = data.other_robots_data[attackable_enemy_id].position.y;
    m_next_msg.goto_parameters.angle = atan2f(m_next_msg.goto_parameters.y - robot.position.y, m_next_msg.goto_parameters.x - robot.position.x);
    m_next_msg.goto_parameters.high_speed = true;
    m_next_msg.goto_parameters.backward = false;
    break;

  case Action::DEFEND:
    /* ... rotate until the enemy is dead or away! */
    m_next_msg.ordered_state = TrajectoryState::ROTATE;
    break;

  default:
    break;
  }

  /* We just hit a wall, but it was planed */
  if (move_dir != -1)
  {
    if (m_walls.HasWallInDirection(ix, iy, move_dir))
    {
      m_state.wall_hits++;
    }
  }

  /* We are now ready with our message, we set this flag to avoid recomputing everything until the message is consumed */
  m_next_msg_valid = true;
}

StrategyToTrajectory Strategy::GetNextMsg() const
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
  m_match_start_ms = millis();

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
        /* There is a vertical wall between (x, y) and (x+1, y) if (x, y)'s east square is NULL */
        m_walls.vertical_walls[x + y * (MAZE_SIZE - 1)] = sqr.eastSquare == nullptr;
      }
      if (y < MAZE_SIZE - 1)
      {
        /* There is a horizontal wall between (x, y) and (x, y+1) if (x, y)'s north square is NULL */
        m_walls.horizontal_walls[x + y * MAZE_SIZE] = sqr.northSquare == nullptr;
      }
    }
  }

  m_maze_initialized = true;
}

void Strategy::UpdateMaze()
{
  /* Walls are never updated, but rewards are. This makes sure the robot can go back on rewards
   * it thought it got, but actually didn't */
  for (size_t x = 0; x < MAZE_SIZE; x++)
  {
    for (size_t y = 0; y < MAZE_SIZE; y++)
    {
      MazeSquare sqr = m_gladiator->maze->getSquare(x, y);
      m_state.rewards[x + y * MAZE_SIZE] = sqr.coin.value;
    }
  }
}
