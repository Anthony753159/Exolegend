
#pragma once

#include <vector>
#include <queue>
#include <array>
#include <algorithm>

#include "gladiator.h"

#include "common.hpp"
#include "comms.hpp"

#define SEARCH_DEPTH 200
#define SEARCH_TIME 20
#define SEARCH_PATHS 256

Action MonteCarloTreeSearch(const GameState &start, Gladiator *gladiator, size_t playing_robot);
