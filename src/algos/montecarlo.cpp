
#include "montecarlo.hpp"

#include <map>

Action MonteCarloTreeSearch(const GameState &start, Gladiator *gladiator)
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

  best_scores_by_action[Action::MOVE_NORTH] = 0.0f;
  best_scores_by_action[Action::MOVE_EAST] = 0.0f;
  best_scores_by_action[Action::MOVE_SOUTH] = 0.0f;
  best_scores_by_action[Action::MOVE_WEST] = 0.0f;

  /* Search N times */

  for (int i = 0; i < SEARCH_PATHS; i++)
  {
    Action first_action = start.GetRandomAction(Action::UNDEFINED);
    GameState current_state = start;
    std::optional<GameState> next_state = start.ApplyAction(first_action);
    Action previous_action = first_action;

    if (!next_state.has_value())
    {
      continue;
    }

    /* Search until depth or game over */

    for (int j = 0; j < SEARCH_DEPTH && next_state.has_value(); j++)
    {
      current_state = next_state.value();
      if (current_state.IsGoal())
      {
        break;
      }

      Action next_action = current_state.GetRandomAction(previous_action);
      next_state = current_state.ApplyAction(next_action);
      previous_action = next_action;
    }

    /* Backpropagate score */
    counts_by_action[first_action]++;
    scores_by_action[first_action] += current_state.rewards_we_got;
    if (current_state.rewards_we_got > best_scores_by_action[first_action])
    {
      best_scores_by_action[first_action] = current_state.rewards_we_got;
    }
  }

  /* Return best action */

  float best_mean_score = -1000.0f;
  float best_score = -1000.0f;
  Action best_action = Action::UNDEFINED;

  for (auto pair : scores_by_action)
  {
    if (best_scores_by_action[pair.first] > best_score)
    {
      best_score = best_scores_by_action[pair.first];
      // best_action = pair.first;
    }

    if (counts_by_action[pair.first] > 0)
    {
      float mean_score = scores_by_action[pair.first] / (1.0f * counts_by_action[pair.first]);
      if (mean_score > best_mean_score)
      {
        best_mean_score = mean_score;
        best_action = pair.first;
      }
    }
  }

  gladiator->log("Best mean score: %f, Best score: %f, Action: %d", best_mean_score, best_score, best_action);

  return best_action;
}