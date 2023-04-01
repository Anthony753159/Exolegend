
#pragma once

#include <vector>
#include <queue>
#include <array>
#include <algorithm>

#include "gladiator.h"

#include "common.hpp"
#include "comms.hpp"

#define SEARCH_DEPTH 30
#define SEARCH_TIME 120
#define SEARCH_PATHS 4096

Action MonteCarloTreeSearch(const GameState &start, Gladiator *gladiator);
