
#include "maze.hpp"

#include "gamestate.hpp"

bool MazeWalls::HasWallInDirection(int8_t x, int8_t y, int8_t direction) const
{
  switch (direction)
  {
  case Direction::NORTH:
    return horizontal_walls[x + y * MAZE_SIZE];
  case Direction::EAST:
    return vertical_walls[x + y * (MAZE_SIZE - 1)];
  case Direction::SOUTH:
    return horizontal_walls[x + (y - 1) * MAZE_SIZE];
  case Direction::WEST:
    return vertical_walls[x - 1 + y * (MAZE_SIZE - 1)];
  }

  return false;
}
