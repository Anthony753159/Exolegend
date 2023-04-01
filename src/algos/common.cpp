#include "common.hpp"

#include <cstdio>
#include <random>

bool MazeWalls::HasPathToCenter(int8_t x, int8_t y, int8_t r)
{
  int8_t cx = MAZE_SIZE / 2;
  int8_t cy = MAZE_SIZE / 2;

  if (x == cx && y == cy)
  {
    return true;
  }

  for (size_t i = 0; i < MAZE_SIZE * MAZE_SIZE; i++)
  {
    visit_map[i] = false;
  }

  return LookForPath(x, y, cx, cy, r);
}

bool MazeWalls::LookForPath(int8_t x, int8_t y, int8_t tx, int8_t ty, int8_t r)
{
  if (x == tx && y == ty)
  {
    return true;
  }

  visit_map[x + y * MAZE_SIZE] = true;

  int8_t nx = x + 1;
  int8_t ny = y;
  if (nx < MAZE_SIZE - r && nx >= r && ny < MAZE_SIZE - r && !visit_map[nx + ny * MAZE_SIZE] && ny >= r && !IsWall(x, y, 1) && LookForPath(nx, ny, tx, ty, r))
  {
    return true;
  }

  nx = x - 1;
  ny = y;
  if (nx < MAZE_SIZE - r && nx >= r && ny < MAZE_SIZE - r && ny >= r && !visit_map[nx + ny * MAZE_SIZE] && !IsWall(x, y, 3) && LookForPath(nx, ny, tx, ty, r))
  {
    return true;
  }

  nx = x;
  ny = y + 1;
  if (nx < MAZE_SIZE - r && nx >= r && ny < MAZE_SIZE - r && ny >= r && !visit_map[nx + ny * MAZE_SIZE] && !IsWall(x, y, 0) && LookForPath(nx, ny, tx, ty, r))
  {
    return true;
  }

  nx = x;
  ny = y - 1;
  if (nx < MAZE_SIZE - r && nx >= r && ny < MAZE_SIZE - r && ny >= r && !visit_map[nx + ny * MAZE_SIZE] && !IsWall(x, y, 2) && LookForPath(nx, ny, tx, ty, r))
  {
    return true;
  }

  return false;
}

GameState::GameState() = default;

GameState::GameState(const GameState &other)
{
  time = other.time;
  maze_retract = other.maze_retract;
  sum_of_rewards = other.sum_of_rewards;
  pos = other.pos;
  direction = other.direction;
  remaining_slow_down = other.remaining_slow_down;
  rewards_we_got = other.rewards_we_got;
  wall_hits = other.wall_hits;

  for (size_t i = 0; i < MAZE_SIZE * MAZE_SIZE; i++)
  {
    rewards[i] = other.rewards[i];
  }
}

void GameState::SetTime(float t)
{
  remaining_slow_down -= t - time;
  time = t;
  maze_retract = static_cast<int>((time + 4.0f) / RETRACT_PERIOD);
}

bool GameState::operator==(const GameState &other) const
{
  bool same_rewards = true;
  for (size_t i = 0; i < MAZE_SIZE * MAZE_SIZE; i++)
  {
    if (rewards[i] != other.rewards[i])
    {
      same_rewards = false;
      break;
    }
  }

  return maze_retract == other.maze_retract && pos.x == other.pos.x && pos.y == other.pos.y && direction == other.direction && remaining_slow_down == other.remaining_slow_down && same_rewards;
}

bool GameState::IsGoal() const
{
  return time >= GAME_DURATION;
}

Vec2 GameState::GetNorth() const
{
  Vec2 north = pos;
  north.y++;
  return north;
}

Vec2 GameState::GetEast() const
{
  Vec2 east = pos;
  east.x++;
  return east;
}

Vec2 GameState::GetSouth() const
{
  Vec2 south = pos;
  south.y--;
  return south;
}

Vec2 GameState::GetWest() const
{
  Vec2 west = pos;
  west.x--;
  return west;
}

Vec2 GameState::GetForward() const
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

Vec2 GameState::GetBackward() const
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

int8_t GameState::GetDirectionRight() const
{
  return (direction + 1) % 4;
}

int8_t GameState::GetDirectionLeft() const
{
  return (direction + 3) % 4;
}

int8_t GameState::GetDirectionBackward() const
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

std::optional<GameState> GameState::ApplyAction(Action action) const
{
  GameState new_state{*this};

  bool hit_a_wall = false;
  float action_time = 0.0f;

  switch (action)
  {
  case MOVE_NORTH:
    new_state.pos = GetNorth();
    new_state.direction = 0;
    action_time = TIME_ONE_CELL_FULL_SPEED + (direction == 0 || direction == 2 ? 0 : TIME_TURN);
    break;
  case MOVE_EAST:
    new_state.pos = GetEast();
    new_state.direction = 1;
    action_time = TIME_ONE_CELL_FULL_SPEED + (direction == 1 || direction == 3 ? 0 : TIME_TURN);
    break;
  case MOVE_SOUTH:
    new_state.pos = GetSouth();
    new_state.direction = 2;
    action_time = TIME_ONE_CELL_FULL_SPEED + (direction == 0 || direction == 2 ? 0 : TIME_TURN);
    break;
  case MOVE_WEST:
    new_state.pos = GetWest();
    new_state.direction = 3;
    action_time = TIME_ONE_CELL_FULL_SPEED + (direction == 1 || direction == 3 ? 0 : TIME_TURN);
    break;
  case UNDEFINED:
    break;
  }

  new_state.SetTime(time + action_time * (remaining_slow_down > 0.0f ? SLOWDOWN_FACTOR : 1.0f));

  if (!IsInsideBounds(new_state.pos))
  {
    return std::nullopt;
  }

  // bool above_before = pos.y >= MAZE_SIZE / 2;
  // bool above_after = new_state.pos.y >= MAZE_SIZE / 2;
  // if (above_before != above_after)
  // {
  //   return std::nullopt;
  // }

  hit_a_wall = MazeWalls::GetInstance()->IsWall(pos.x, pos.y, new_state.direction);
  if (hit_a_wall)
  {
    return std::nullopt;
    new_state.remaining_slow_down = BASE_SLOWDOWN_DURATION + new_state.wall_hits * PER_HIT_SLOWDOWN_DURATION;
    new_state.wall_hits++;
  }

  float reward_there = rewards[new_state.pos.x + new_state.pos.y * MAZE_SIZE];
  new_state.rewards[new_state.pos.x + new_state.pos.y * MAZE_SIZE] = 0;
  new_state.sum_of_rewards -= reward_there;
  new_state.rewards_we_got += reward_there;

  return new_state;
}