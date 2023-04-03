#pragma once

#include <stdint.h>

#define MAZE_SIZE 14

class MazeWalls
{
public:
  bool HasWallInDirection(int8_t x, int8_t y, int8_t direction) const;

  /* Walls we encounter when we walk horizontally */
  bool vertical_walls[(MAZE_SIZE - 1) * MAZE_SIZE] = {0};

  /* Walls we encounter when we walk vertically */
  bool horizontal_walls[MAZE_SIZE * (MAZE_SIZE - 1)] = {0};
};
