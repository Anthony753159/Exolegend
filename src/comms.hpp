#pragma once

#define MAZE_SIZE 14

struct TrajectoryMsg
{
  enum State
  {
    IDLE,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    ROTATE,
    MOVE_DISTANCE,
    GOTO
  };

  enum Order
  {
    UNDEFINED,
    ORDER_GOTO,
    ORDER_SET_STATE,
    ORDER_ROTATE,
    ORDER_MOVE_DISTANCE
  };

  Order order = UNDEFINED;

  // ORDER_GOTO
  float goto_x;
  float goto_y;

  // ORDER_SET_STATE
  State state;

  // ORDER_ROTATE
  float angle;

  // MOVE_DISTANCE
  float distance;
  bool forward;
};
