#pragma once

#include <vector>
#include <queue>
#include <array>
#include <algorithm>

#include "gladiator.h"

#include "common.hpp"
#include "comms.hpp"

#define MAX_NODES 1024 * 3

#define TIME_ONE_CELL_FULL_SPEED 1.0f
#define TIME_TURN 0.5f
#define SLOWDOWN_FACTOR 10.0f
#define SLOWDOWN_DURATION 4.0f

struct Neighboring
{
  bool has_forward;
  float cost_forward;
  GameState forward;

  bool has_backward;
  float cost_backward;
  GameState backward;

  GameState turn_left;
  float cost_turn_left;

  GameState turn_right;
  float cost_turn_right;
};

struct AStarNode
{
  Action action;
  size_t parent_index;
  GameState state;
  double cost;
  double priority;

  AStarNode();
  AStarNode(Action action, size_t parent_index, const GameState &state, double cost);

  bool operator<(const AStarNode &other) const;
  double CalculateHeuristic() const;
  void GetNeighbors(Neighboring *neighbors) const;
};

Action AStar(const GameState &start, Gladiator *gladiator);
