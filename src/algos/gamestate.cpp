#include "gamestate.hpp"

#include <cstdio>
#include <random>

#include "gladiator.h"
#include "maze.hpp"

bool Orthogonal(Direction a, Direction b)
{
  return ((a == Direction::NORTH || a == Direction::SOUTH) && (b == Direction::EAST || b == Direction::WEST)) ||
         ((a == Direction::EAST || a == Direction::WEST) && (b == Direction::NORTH || b == Direction::SOUTH));
}

bool Opposite(Direction a, Direction b)
{
  return (a == Direction::NORTH && b == Direction::SOUTH) ||
         (a == Direction::SOUTH && b == Direction::NORTH) ||
         (a == Direction::EAST && b == Direction::WEST) ||
         (a == Direction::WEST && b == Direction::EAST);
}

Direction ActionToDirection(Action action)
{
  switch (action)
  {
  case MOVE_NORTH:
    return Direction::NORTH;
  case MOVE_EAST:
    return Direction::EAST;
  case MOVE_SOUTH:
    return Direction::SOUTH;
  case MOVE_WEST:
    return Direction::WEST;
  default:
    return Direction::NORTH;
  }
}

bool IsMovement(Action action)
{
  return action == MOVE_NORTH || action == MOVE_EAST || action == MOVE_SOUTH || action == MOVE_WEST;
}

float DirectionToAngle(Direction direction)
{
  switch (direction)
  {
  case Direction::NORTH:
    return M_PI / 2.0f;
  case Direction::EAST:
    return 0.0f;
  case Direction::SOUTH:
    return -M_PI / 2.0f;
  case Direction::WEST:
    return M_PI;
  default:
    return 0.0f;
  }
}

Direction AngleToDirection(float angle)
{
  if (angle >= -M_PI / 4.0f && angle < M_PI / 4.0f)
    return Direction::EAST;
  else if (angle >= M_PI / 4.0f && angle < 3.0f * M_PI / 4.0f)
    return Direction::NORTH;
  else if (angle >= -3.0f * M_PI / 4.0f && angle < -M_PI / 4.0f)
    return Direction::SOUTH;
  else
    return Direction::WEST;
}

Vec2i Vec2i::operator+(const Direction &direction) const
{
  switch (direction)
  {
  case Direction::NORTH:
    return {x, static_cast<int8_t>(y + 1)};
  case Direction::EAST:
    return {static_cast<int8_t>(x + 1), y};
  case Direction::SOUTH:
    return {x, static_cast<int8_t>(y - 1)};
  case Direction::WEST:
    return {static_cast<int8_t>(x - 1), y};
  default:
    return {x, y};
  }
}

GameState::GameState() = default;

GameState::GameState(const GameState &other)
{
  time = other.time;
  maze_retracts = other.maze_retracts;
  pos = other.pos;
  direction = other.direction;
  remaining_slow_down = other.remaining_slow_down;
  rewards_we_got = other.rewards_we_got;
  wall_hits = other.wall_hits;

  for (size_t i = 0; i < MAZE_SIZE * MAZE_SIZE; i++)
  {
    rewards[i] = other.rewards[i];
    visits[i] = other.visits[i];
  }
}

void GameState::UpdateTime(float t)
{
  remaining_slow_down -= t - time;
  time = t;
  maze_retracts = static_cast<int>((time + MAZE_RETRACT_ANTICIPATION_TIME) / MAZE_RETRACT_PERIOD);
}

bool GameState::IsGameOver() const
{
  return time >= GAME_DURATION;
}

bool GameState::IsInsideBounds(const Vec2i &vec) const
{
  return vec.x >= maze_retracts &&
         vec.x < MAZE_SIZE - maze_retracts &&
         vec.y >= maze_retracts &&
         vec.y < MAZE_SIZE - maze_retracts;
}

Action GameState::GetRandomMoveAction() const
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
  default:
    return MOVE_WEST;
  }
}

std::optional<GameState> GameState::ApplyAction(Action action, bool first_action, const MazeWalls &walls) const
{
  /* Copying current state */
  GameState new_state{*this};

  new_state.direction = ActionToDirection(action);
  new_state.pos = pos + new_state.direction;

  if (!IsInsideBounds(new_state.pos))
  {
    /* Action is invalid if it puts the robot outside of bounds */
    return std::nullopt;
  }

  float action_time = TIME_TO_TRAVEL_ONE_CELL + (Orthogonal(direction, new_state.direction) ? ADDITIONAL_TIME_TO_TURN : 0.0f);
  new_state.UpdateTime(time + action_time * (remaining_slow_down > 0.0f ? SLOWDOWN_FACTOR : 1.0f));

  new_state.visits[new_state.pos.x + new_state.pos.y * MAZE_SIZE]++;

  if (walls.HasWallInDirection(pos.x, pos.y, new_state.direction))
  {
    bool can_cross_walls = !FORBID_WALLS;
    can_cross_walls |= first_action && visits[pos.x + pos.y * MAZE_SIZE] >= N_VISITS_BEFORE_CROSSING_WALLS;
    if (!can_cross_walls)
    {
      /* Action is invalid if it crosses a wall and it is forbidden */
      return std::nullopt;
    }

    new_state.remaining_slow_down = BASE_SLOWDOWN_DURATION + new_state.wall_hits * PER_HIT_SLOWDOWN_DURATION;
    new_state.wall_hits++;
  }

  /* Picking up the reward at current location */
  float reward_there = rewards[new_state.pos.x + new_state.pos.y * MAZE_SIZE];
  new_state.rewards[new_state.pos.x + new_state.pos.y * MAZE_SIZE] = 0;
  new_state.rewards_we_got += reward_there;

  return new_state;
}