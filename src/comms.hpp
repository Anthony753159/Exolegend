#pragma once

struct TrajectoryMsg
{
  enum State
  {
    IDLE,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
  };

  enum Order
  {
    UNDEFINED,
    GOTO,
    SET_STATE,
  };

  Order order = UNDEFINED;

  // GOTO
  float goto_x;
  float goto_y;

  // SET_STATE
  State state;
};