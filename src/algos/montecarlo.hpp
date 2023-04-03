#pragma once

#include "gamestate.hpp"
#include "maze.hpp"

#define SEARCH_DEPTH 300
#define SEARCH_PATHS 256

Action MonteCarloTreeSearch(const GameState &start_state, const MazeWalls &walls);
