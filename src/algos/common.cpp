#include "common.hpp"

#include <cstdio>
#include <random>

GameState::GameState() = default;

GameState::GameState(const GameState &other)
{
  maze_retract = other.maze_retract;
  sum_of_rewards = other.sum_of_rewards;
  pos = other.pos;
  direction = other.direction;
  remaining_slow_down = other.remaining_slow_down;
  rewards_we_got = other.rewards_we_got;

  for (size_t i = 0; i < MAZE_SIZE * MAZE_SIZE; i++)
  {
    rewards[i] = other.rewards[i];
  }
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
  return sum_of_rewards <= 0 || maze_retract >= 7;
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
  int8_t rand_index = 0;
  if (previous_action == TURN_LEFT || previous_action == TURN_RIGHT)
  {
    rand_index = random() % 2;
  }
  else
  {
    rand_index = random() % 4;
  }

  switch (rand_index)
  {
  case 0:
    return MOVE_FORWARD;
  case 1:
    return MOVE_BACKWARD;
  case 2:
    return TURN_LEFT;
  case 3:
    return TURN_RIGHT;
  }

  return UNDEFINED;
}

std::optional<GameState> GameState::ApplyAction(Action action) const
{
  GameState new_state{*this};

  bool hit_a_wall = false;
  float action_time = 0.0f;

  switch (action)
  {
  case TURN_LEFT:
    new_state.direction = GetDirectionLeft();
    action_time = TIME_TURN;
    break;
  case TURN_RIGHT:
    new_state.direction = GetDirectionRight();
    action_time = TIME_TURN;
    break;
  case MOVE_FORWARD:
    new_state.pos = GetForward();
    hit_a_wall = MazeWalls::GetInstance()->IsWall(pos.x, pos.y, direction);
    action_time = TIME_ONE_CELL_FULL_SPEED;
    break;
  case MOVE_BACKWARD:
    new_state.pos = GetBackward();
    hit_a_wall = MazeWalls::GetInstance()->IsWall(pos.x, pos.y, GetDirectionBackward());
    action_time = TIME_ONE_CELL_FULL_SPEED;
    break;
  case UNDEFINED:
    break;
  }

  if (!IsInsideBounds(new_state.pos))
  {
    return std::nullopt;
  }

  if (hit_a_wall)
  {
    new_state.remaining_slow_down = BASE_SLOWDOWN_DURATION + new_state.wall_hits * PER_HIT_SLOWDOWN_DURATION;
    new_state.wall_hits++;
  }

  new_state.time += action_time * (remaining_slow_down > 0.0f ? SLOWDOWN_FACTOR : 1.0f);
  new_state.maze_retract = static_cast<int>(std::floor(new_state.time / RETRACT_PERIOD));

  float reward_there = rewards[new_state.pos.x + new_state.pos.y * MAZE_SIZE];
  new_state.rewards[new_state.pos.x + new_state.pos.y * MAZE_SIZE] = 0;
  new_state.sum_of_rewards -= reward_there;
  new_state.rewards_we_got += reward_there;

  return new_state;
}