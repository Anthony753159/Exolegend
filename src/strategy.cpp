#include "strategy.hpp"

#include <math.h>

#include "algos/common.hpp"
#include "algos/astar.hpp"
#include "algos/montecarlo.hpp"

Strategy::Strategy(Gladiator *gladiator) : m_gladiator(gladiator)
{
}

Strategy::~Strategy()
{
}

TrajectoryMsg Strategy::Update(const RobotData &data)
{
  delay(1000);
  m_gladiator->log("Updating strategy");

  if (!m_maze_initialized)
  {
    InitMaze();
  }
  UpdateMaze();

  TrajectoryMsg msg;

  GameState start;
  start.maze_retract = 0;

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

  start.pos = {ix, iy};
  start.direction = idir;
  start.remaining_slow_down = 0.0f;
  start.maze_retract = 0;
  start.rewards_we_got = 0;
  start.wall_hits = 0;
  start.time = 0;

  /*
    QUESTIONS:
      How do we get the time in game?
        > Use system time [Millis]
      How do we know for how long the slowdown will happen?
        > 4s + 1s per wall crossed
      How do we know how much the maze retracted, and when it will retract next?
        > once every 20s
  */

  for (size_t i = 0; i < MAZE_SIZE * MAZE_SIZE; i++)
  {
    start.rewards[i] = m_rewards[i];
    start.sum_of_rewards += m_rewards[i];
  }

  Action action = MonteCarloTreeSearch(start, m_gladiator);

  switch (action)
  {
  case Action::MOVE_FORWARD:
    m_gladiator->log("MOVE_FORWARD");
    msg.order = TrajectoryMsg::ORDER_MOVE_DISTANCE;
    msg.distance = m_square_size;
    msg.forward = true;
    break;
  case Action::MOVE_BACKWARD:
    m_gladiator->log("MOVE_BACKWARD");
    msg.order = TrajectoryMsg::ORDER_MOVE_DISTANCE;
    msg.distance = m_square_size;
    msg.forward = false;
    break;
  case Action::TURN_LEFT:
    m_gladiator->log("TURN_LEFT");
    msg.order = TrajectoryMsg::ORDER_ROTATE;
    msg.angle = (1 - start.GetDirectionLeft()) * M_PI / 2.0f;
    break;
  case Action::TURN_RIGHT:
    m_gladiator->log("TURN_RIGHT");
    msg.order = TrajectoryMsg::ORDER_ROTATE;
    msg.angle = (1 - start.GetDirectionRight()) * M_PI / 2.0f;
    break;
  case Action::UNDEFINED:
    break;
  }

  return msg;
}

void Strategy::InitMaze()
{
  m_maze_size = m_gladiator->maze->getSize();
  m_square_size = m_gladiator->maze->getSquareSize();

  for (size_t x = 0; x < MAZE_SIZE; x++)
  {
    for (size_t y = 0; y < MAZE_SIZE; y++)
    {
      MazeSquare sqr = m_gladiator->maze->getSquare(x, y);

      m_rewards[x + y * MAZE_SIZE] = sqr.coin.value;

      if (x < MAZE_SIZE - 1)
      {
        MazeWalls::GetInstance()->horizontal_walls[x + y * (MAZE_SIZE - 1)] = sqr.eastSquare != nullptr;
      }
      if (y < MAZE_SIZE - 1)
      {
        MazeWalls::GetInstance()->vertical_walls[x + y * MAZE_SIZE] = sqr.southSquare != nullptr;
      }
    }
  }

  m_maze_initialized = true;
}

void Strategy::UpdateMaze()
{
  for (size_t x = 0; x < MAZE_SIZE; x++)
  {
    for (size_t y = 0; y < MAZE_SIZE; y++)
    {
      MazeSquare sqr = m_gladiator->maze->getSquare(x, y);

      m_rewards[x + y * MAZE_SIZE] = sqr.coin.value;

      if (x < MAZE_SIZE - 1)
      {
        MazeWalls::GetInstance()->horizontal_walls[x + y * (MAZE_SIZE - 1)] = sqr.eastSquare != nullptr;
      }
      if (y < MAZE_SIZE - 1)
      {
        MazeWalls::GetInstance()->vertical_walls[x + y * MAZE_SIZE] = sqr.southSquare != nullptr;
      }
    }
  }
}
