#pragma once

#include <math.h>

#define MAZE_SIZE 14

struct TrajectoryMsg
{
  enum State
  {
    IDLE,
    ROTATE,
    GOTO
  };

  enum Order
  {
    UNDEFINED,
    ORDER_GOTO,
    ORDER_ROTATE,
  };

  Order order = UNDEFINED;

  // ORDER_GOTO
  float goto_x;
  float goto_y;
  float goto_angle;
  bool goto_reverse;
  bool goto_high_speed;
};

float AngleDiffRad(float from, float to);
float Abs(float a);
