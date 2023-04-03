#pragma once

#include <stdint.h>
#include <optional>

#include "maze.hpp"

/* Constants about the game itseld */

#define GAME_DURATION 120.0f
#define MAZE_RETRACT_PERIOD 20.0f
#define N_ROBOTS 4

/* Behavioral parameters */

/* Whether or not we allow the robot to cross walls if it is supposedly more interesting */
#define FORBID_WALLS true
/* Even if FORBID_WALLS is 'true', we can allow to cross walls if we already visited the current cell this many times */
#define N_VISITS_BEFORE_CROSSING_WALLS 3
/* How many seconds before an actual maze retract do we believe the maze to be retracted */
#define MAZE_RETRACT_ANTICIPATION_TIME 2.0f

/* Timing parameters - Used to compute the cost of each action. SLOWDOWN is a bit messed up. */

#define TIME_TO_TRAVEL_ONE_CELL 0.5f
#define ADDITIONAL_TIME_TO_TURN 0.5f
#define SLOWDOWN_FACTOR 10.0f
#define BASE_SLOWDOWN_DURATION 8.0f
#define PER_HIT_SLOWDOWN_DURATION 1.0f

enum Action : uint8_t
{
  UNDEFINED,
  MOVE_NORTH,
  MOVE_EAST,
  MOVE_SOUTH,
  MOVE_WEST,
  ATTACK,
  DEFEND
};

enum Direction : uint8_t
{
  NORTH,
  EAST,
  SOUTH,
  WEST
};

bool Orthogonal(Direction a, Direction b);
bool Opposite(Direction a, Direction b);
Direction ActionToDirection(Action action);
bool IsMovement(Action action);
float DirectionToAngle(Direction direction);
Direction AngleToDirection(float angle);

struct Vec2i
{
  int8_t x;
  int8_t y;

  Vec2i operator+(const Direction &direction) const;
};

/*
 * Represents a simplified state of the game:
 * - We discretize the space into a grid of cells
 * - The robot is always at the center of a cell, and facing a perfect cardinal direction
 * - Only the current robot is taken into account
 *
 * Additional things were added to the game state to improve the strategy.
 */
struct GameState
{
  /* Game state */
  float time = 0.0f;
  int8_t maze_retracts = 0;
  float rewards[MAZE_SIZE * MAZE_SIZE] = {0};

  /* Robot state */
  Vec2i pos = {0, 0};
  Direction direction = Direction::NORTH;
  float remaining_slow_down = 0.0f;
  int wall_hits = 0;

  /* Additional data */
  float rewards_we_got = 0;                   // Total rewards accumulated by the robot
  int8_t visits[MAZE_SIZE * MAZE_SIZE] = {0}; // Number of times we visited each cell

  GameState();
  GameState(const GameState &other); // Copy constructor

  void UpdateTime(float t);

  bool IsGameOver() const;

  bool IsInsideBounds(const Vec2i &vec) const;
  Action GetRandomMoveAction() const;

  /* Tries to apply the given Action to the GameState, producing a subsequent GameState (or nullopt if the Action is forbidden) */
  std::optional<GameState> ApplyAction(Action action, bool first_action, const MazeWalls &walls) const;
};