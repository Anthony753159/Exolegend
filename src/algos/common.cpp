#include "common.hpp"

#include <cstdio>
#include <random>

float Vec2::Distance(const Vec2 &other) const
{
  return sqrtf(1.0f * ((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y)));
}

GameState::GameState() = default;

GameState::GameState(const GameState &other)
{
  time = other.time;
  maze_retract = other.maze_retract;
  sum_of_rewards = other.sum_of_rewards;
  rewards_we_got = other.rewards_we_got;

  for (size_t i = 0; i < N_ROBOTS; i++)
  {
    robots[i].pos.x = other.robots[i].pos.x;
    robots[i].pos.y = other.robots[i].pos.y;
    robots[i].direction = other.robots[i].direction;
    robots[i].remaining_slow_down = other.robots[i].remaining_slow_down;
    robots[i].wall_hits = other.robots[i].wall_hits;
    robots[i].current_action = other.robots[i].current_action;
    robots[i].next_action_time = other.robots[i].next_action_time;
    robots[i].ally = other.robots[i].ally;
  }

  for (size_t i = 0; i < MAZE_SIZE * MAZE_SIZE; i++)
  {
    rewards[i] = other.rewards[i];
  }
}

void GameState::SetTime(float t)
{
  for (size_t i = 0; i < N_ROBOTS; i++)
  {
    robots[i].remaining_slow_down -= t - time;
  }

  time = t;
  maze_retract = static_cast<int>((time + 4.0f) / RETRACT_PERIOD);
}

bool GameState::IsGoal() const
{
  return time >= GAME_DURATION;
}

Vec2 RobotState::GetNorth() const
{
  Vec2 north = pos;
  north.y++;
  return north;
}

Vec2 RobotState::GetEast() const
{
  Vec2 east = pos;
  east.x++;
  return east;
}

Vec2 RobotState::GetSouth() const
{
  Vec2 south = pos;
  south.y--;
  return south;
}

Vec2 RobotState::GetWest() const
{
  Vec2 west = pos;
  west.x--;
  return west;
}

Vec2 RobotState::GetForward() const
{
  Vec2 forward = pos;

  switch (direction)
  {
  case 0: // North
    forward.y++;
    break;
  case 1: // East
    forward.x++;
    break;
  case 2: // South
    forward.y--;
    break;
  case 3: // West
    forward.x--;
    break;
  }

  return forward;
}

Vec2 RobotState::GetBackward() const
{
  Vec2 backward = pos;

  switch (direction)
  {
  case 0:
    backward.y--;
    break;
  case 1:
    backward.x--;
    break;
  case 2:
    backward.y++;
    break;
  case 3:
    backward.x++;
    break;
  }

  return backward;
}

int8_t RobotState::GetDirectionRight() const
{
  return (direction + 1) % 4;
}

int8_t RobotState::GetDirectionLeft() const
{
  return (direction + 3) % 4;
}

int8_t RobotState::GetDirectionBackward() const
{
  return (direction + 2) % 4;
}

bool GameState::IsInsideBounds(const Vec2 &vec) const
{
  return vec.x >= maze_retract &&
         vec.x < MAZE_SIZE - maze_retract &&
         vec.y >= maze_retract &&
         vec.y < MAZE_SIZE - maze_retract;
}

Action GameState::GetRandomAction(Action previous_action) const
{
  int8_t rand_index = random() % 4;

  switch (rand_index)
  {
  case 0:
    return MOVE_NORTH;
  case 1:
    return MOVE_EAST;
  case 2:
    return MOVE_SOUTH;
  case 3:
    return MOVE_WEST;
  }

  return MOVE_NORTH;
}

std::optional<GameState> GameState::ApplyAction(Action action, size_t playing_robot) const
{
  GameState new_state{*this};

  bool hit_a_wall = false;
  float action_time = 0.0f;

  switch (action)
  {
  case MOVE_NORTH:
    new_state.robots[playing_robot].pos = new_state.robots[playing_robot].GetNorth();
    new_state.robots[playing_robot].direction = 0;
    action_time = TIME_ONE_CELL_FULL_SPEED + (new_state.robots[playing_robot].direction == 0 || new_state.robots[playing_robot].direction == 2 ? 0 : TIME_TURN);
    break;
  case MOVE_EAST:
    new_state.robots[playing_robot].pos = new_state.robots[playing_robot].GetEast();
    new_state.robots[playing_robot].direction = 1;
    action_time = TIME_ONE_CELL_FULL_SPEED + (new_state.robots[playing_robot].direction == 1 || new_state.robots[playing_robot].direction == 3 ? 0 : TIME_TURN);
    break;
  case MOVE_SOUTH:
    new_state.robots[playing_robot].pos = new_state.robots[playing_robot].GetSouth();
    new_state.robots[playing_robot].direction = 2;
    action_time = TIME_ONE_CELL_FULL_SPEED + (new_state.robots[playing_robot].direction == 0 || new_state.robots[playing_robot].direction == 2 ? 0 : TIME_TURN);
    break;
  case MOVE_WEST:
    new_state.robots[playing_robot].pos = new_state.robots[playing_robot].GetWest();
    new_state.robots[playing_robot].direction = 3;
    action_time = TIME_ONE_CELL_FULL_SPEED + (new_state.robots[playing_robot].direction == 1 || new_state.robots[playing_robot].direction == 3 ? 0 : TIME_TURN);
    break;
  case UNDEFINED:
    break;
  }

  if (!IsInsideBounds(new_state.robots[playing_robot].pos))
  {
    return std::nullopt;
  }

  bool above_before = robots[playing_robot].pos.y >= MAZE_SIZE / 2;
  bool above_after = new_state.robots[playing_robot].pos.y >= MAZE_SIZE / 2;
  if (above_before != above_after)
  {
    return std::nullopt;
  }

  float next_time = GAME_DURATION;
  for (size_t i = 0; i < N_ROBOTS; i++)
  {
    if (i != playing_robot)
    {
      if (new_state.robots[i].pos.x == new_state.robots[playing_robot].pos.x && new_state.robots[i].pos.y == new_state.robots[playing_robot].pos.y)
      {
        return std::nullopt;
      }
    }
    else
    {
      new_state.robots[i].current_action = action;
      new_state.robots[i].next_action_time = time + action_time * (new_state.robots[i].remaining_slow_down > 0.0f ? SLOWDOWN_FACTOR : 1.0f);
    }

    if (new_state.robots[i].next_action_time < next_time)
    {
      next_time = new_state.robots[i].next_action_time;
    }
  }

  new_state.SetTime(next_time);

  hit_a_wall = MazeWalls::GetInstance()->IsWall(robots[playing_robot].pos.x, robots[playing_robot].pos.y, new_state.robots[playing_robot].direction);
  if (hit_a_wall)
  {
    if (time < 40)
    {
      return std::nullopt;
    }
    new_state.robots[playing_robot].remaining_slow_down = BASE_SLOWDOWN_DURATION + new_state.robots[playing_robot].wall_hits * PER_HIT_SLOWDOWN_DURATION;
    new_state.robots[playing_robot].wall_hits++;
  }

  float reward_there = rewards[new_state.robots[playing_robot].pos.x + new_state.robots[playing_robot].pos.y * MAZE_SIZE];
  new_state.rewards[new_state.robots[playing_robot].pos.x + new_state.robots[playing_robot].pos.y * MAZE_SIZE] = 0;
  new_state.sum_of_rewards -= reward_there;
  new_state.rewards_we_got += reward_there * (new_state.robots[playing_robot].ally ? 1.0f : -1.0f);

  return new_state;
}

std::optional<GameState> GameState::ApplyAction(Action action) const
{
  size_t playing_robot = 0;
  float minimum_action_time = 1000000.0f;
  for (size_t i = 0; i < N_ROBOTS; i++)
  {
    if (robots[i].next_action_time < minimum_action_time)
    {
      minimum_action_time = robots[i].next_action_time;
      playing_robot = i;
    }
  }

  return ApplyAction(action, playing_robot);
}