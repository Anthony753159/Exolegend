
#pragma once

#include <vector>
#include <queue>
#include <array>
#include <algorithm>

#include "gladiator.h"

#include "common.hpp"
#include "utils.hpp"

#define SEARCH_DEPTH 300
#define SEARCH_PATHS 256

Action MonteCarloTreeSearch(const GameState &start, Gladiator *gladiator);
