
#include "montecarlo.hpp"

#include <algorithm>
#include <map>
#include <math.h>

#include "utils.hpp"

Action MonteCarloTreeSearch(const GameState &start_state, const MazeWalls &walls)
{
  std::map<Action, int> counts_by_action;
  std::map<Action, float> scores_by_action;
  std::map<Action, float> best_scores_by_action;

  counts_by_action[Action::MOVE_NORTH] = 0;
  counts_by_action[Action::MOVE_EAST] = 0;
  counts_by_action[Action::MOVE_SOUTH] = 0;
  counts_by_action[Action::MOVE_WEST] = 0;

  scores_by_action[Action::MOVE_NORTH] = 0.0f;
  scores_by_action[Action::MOVE_EAST] = 0.0f;
  scores_by_action[Action::MOVE_SOUTH] = 0.0f;
  scores_by_action[Action::MOVE_WEST] = 0.0f;

  /* Simulate many scenarios from current state */
  for (int i = 0; i < SEARCH_PATHS; i++)
  {
    Action first_action = start_state.GetRandomMoveAction();
    GameState current_state = start_state;
    std::optional<GameState> next_state = start_state.ApplyAction(first_action, true, walls);

    if (!next_state.has_value())
    {
      /* First action was invalid, don't even count that scenario */
      continue;
    }

    /* Search until maximum depth, an invalid gamestate, or a gameover situation */
    for (int j = 0; j < SEARCH_DEPTH && next_state.has_value(); j++)
    {
      if (!next_state.has_value())
      {
        break;
      }

      current_state = next_state.value();
      if (current_state.IsGameOver())
      {
        break;
      }

      Action next_action = current_state.GetRandomMoveAction();
      next_state = current_state.ApplyAction(next_action, false, walls);
    }
    /* NOTE: 'current_state' always contains the last valid state */

    /* Accumulate scores based on the first action taken in the scenario */
    counts_by_action[first_action]++;
    scores_by_action[first_action] += current_state.rewards_we_got;
  }

  /* Return the action that led to the best scenarios in average */

  float best_mean_score = nanf("");
  Action best_action = Action::UNDEFINED;

  for (auto pair : scores_by_action)
  {
    if (counts_by_action[pair.first] > 0)
    {
      float mean_score = pair.second / (1.0f * counts_by_action[pair.first]);
      if (isnan(best_mean_score) || mean_score > best_mean_score)
      {
        best_mean_score = mean_score;
        best_action = pair.first;
      }
    }
  }

  return best_action;
}