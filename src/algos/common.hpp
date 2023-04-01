#pragma once

#include <stdint.h>
#include <optional>

#include "comms.hpp"

#define GAME_DURATION 120.0f
#define TIME_ONE_CELL_FULL_SPEED 1.0f
#define TIME_TURN 1.0f
#define RETRACT_PERIOD 20.0f
#define SLOWDOWN_FACTOR 10.0f
#define BASE_SLOWDOWN_DURATION 4.0f
#define PER_HIT_SLOWDOWN_DURATION 1.0f

struct MazeWalls
{
  static MazeWalls *GetInstance()
  {
    static MazeWalls instance;
    return &instance;
  }

  /* Walls we encounter when we walk horizontally */
  bool horizontal_walls[(MAZE_SIZE - 1) * MAZE_SIZE] = {0};

  /* Walls we encounter when we walk vertically */
  bool vertical_walls[MAZE_SIZE * (MAZE_SIZE - 1)] = {0};

  bool IsWall(int8_t x, int8_t y, int8_t direction) const
  {
    switch (direction)
    {
    case 0: // North
      return vertical_walls[x + y * MAZE_SIZE];
    case 1: // East
      return horizontal_walls[x + y * (MAZE_SIZE - 1)];
    case 2: // South
      return vertical_walls[x + (y - 1) * MAZE_SIZE];
    case 3: // West
      return horizontal_walls[x - 1 + y * (MAZE_SIZE - 1)];
    }

    return false;
  }

private:
  MazeWalls() {}
};

enum Action
{
  UNDEFINED,
  MOVE_NORTH,
  MOVE_EAST,
  MOVE_SOUTH,
  MOVE_WEST,
};

struct Vec2
{
  int8_t x;
  int8_t y;
};

struct GameState
{
  /* State */
  float time = 0.0f;
  int8_t maze_retract = 0;
  float rewards[MAZE_SIZE * MAZE_SIZE] = {0};
  float sum_of_rewards = 0;
  float rewards_we_got = 0;

  Vec2 pos = {0, 0};
  int8_t direction = 0;

  float remaining_slow_down = 0.0f;
  int wall_hits = 0;

  GameState();
  GameState(const GameState &other);

  void SetTime(float t);

  bool operator==(const GameState &other) const;
  bool IsGoal() const;

  Vec2 GetNorth() const;
  Vec2 GetEast() const;
  Vec2 GetSouth() const;
  Vec2 GetWest() const;

  Vec2 GetForward() const;
  Vec2 GetBackward() const;

  int8_t GetDirectionRight() const;
  int8_t GetDirectionLeft() const;
  int8_t GetDirectionBackward() const;

  bool IsInsideBounds(const Vec2 &vec) const;
  Action GetRandomAction(Action previous_action) const;

  std::optional<GameState> ApplyAction(Action action) const;
};