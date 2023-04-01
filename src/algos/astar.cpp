
// #include "astar.hpp"

// AStarNode::AStarNode() = default;

// AStarNode::AStarNode(Action action, size_t parent_index, const GameState &state, double cost)
//     : action(action), parent_index(parent_index), state(state), cost(cost)
// {
//   priority = cost + CalculateHeuristic();
// }

// void AStarNode::GetNeighbors(Neighboring *neighbors) const
// {
//   Vec2 forward = state.GetForward();
//   Vec2 backward = state.GetBackward();

//   if (state.IsInsideBounds(forward))
//   {
//     neighbors->has_forward = true;
//     neighbors->forward = GameState(state);

//     neighbors->forward.pos = forward;
//     float reward_there = state.rewards[forward.x + forward.y * MAZE_SIZE];
//     neighbors->forward.sum_of_rewards = state.sum_of_rewards - reward_there;
//     neighbors->forward.rewards[forward.x + forward.y * MAZE_SIZE] = 0;

//     neighbors->cost_forward = TIME_ONE_CELL_FULL_SPEED * (state.remaining_slow_down > 0 ? SLOWDOWN_FACTOR : 1.0f);
//     neighbors->forward.remaining_slow_down -= neighbors->cost_forward;

//     if (MazeWalls::GetInstance()->IsWall(state.pos.x, state.pos.y, state.direction))
//     {
//       neighbors->forward.remaining_slow_down = SLOWDOWN_DURATION;
//     }
//   }
//   else
//   {
//     neighbors->has_forward = false;
//   }

//   if (state.IsInsideBounds(backward))
//   {
//     neighbors->has_backward = true;
//     neighbors->backward = GameState(state);

//     neighbors->backward.pos = backward;
//     float reward_there = state.rewards[backward.x + backward.y * MAZE_SIZE];
//     neighbors->backward.sum_of_rewards = state.sum_of_rewards - reward_there;
//     neighbors->backward.rewards[backward.x + backward.y * MAZE_SIZE] = 0;

//     neighbors->cost_backward = TIME_ONE_CELL_FULL_SPEED * (state.remaining_slow_down > 0 ? SLOWDOWN_FACTOR : 1.0f);
//     neighbors->backward.remaining_slow_down -= neighbors->cost_backward;

//     if (MazeWalls::GetInstance()->IsWall(state.pos.x, state.pos.y, state.GetDirectionBackward()))
//     {
//       neighbors->backward.remaining_slow_down = BASE_SLOWDOWN_DURATION;
//     }
//   }
//   else
//   {
//     neighbors->has_backward = false;
//   }

//   neighbors->turn_left = GameState(state);
//   neighbors->turn_left.direction = state.GetDirectionLeft();
//   neighbors->cost_turn_left = TIME_TURN * (state.remaining_slow_down > 0 ? SLOWDOWN_FACTOR : 1.0f);
//   neighbors->turn_left.remaining_slow_down -= neighbors->cost_turn_left;

//   neighbors->turn_right = GameState(state);
//   neighbors->turn_right.direction = state.GetDirectionRight();
//   neighbors->cost_turn_right = TIME_TURN * (state.remaining_slow_down > 0 ? SLOWDOWN_FACTOR : 1.0f);
//   neighbors->turn_right.remaining_slow_down -= neighbors->cost_turn_right;
// }

// bool AStarNode::operator<(const AStarNode &other) const
// {
//   return priority > other.priority;
// }

// double AStarNode::CalculateHeuristic() const
// {
//   return state.sum_of_rewards;
// }

// Action AStar(const GameState &start, Gladiator *gladiator)
// {
//   Neighboring neighbors;
//   AStarNode *it;
//   std::priority_queue<AStarNode> open_list;
//   std::array<AStarNode, MAX_NODES> closed_list;
//   size_t closed_list_size = 0;

//   AStarNode start_node{Action::UNDEFINED, 0, start, 0};
//   open_list.push(start_node);

//   while (!open_list.empty())
//   {
//     AStarNode current_node = open_list.top();
//     open_list.pop();

//     if (current_node.state.IsGoal())
//     {
//       while (current_node.parent_index != 0)
//       {
//         current_node = closed_list[current_node.parent_index];
//       }

//       gladiator->log("AStar: %d nodes in closed list", static_cast<int>(closed_list_size));
//       return current_node.action;
//     }

//     it = std::find_if(closed_list.begin(), closed_list.end(), [&](const AStarNode &node)
//                       { return node.state == current_node.state; });

//     if (it != closed_list.end())
//     {
//       continue;
//     }

//     if (closed_list_size < MAX_NODES)
//     {
//       closed_list[closed_list_size] = current_node;
//       closed_list_size++;
//     }
//     else
//     {
//       // Memory limit reached, cannot continue the search.
//       gladiator->log("AStar: Memory limit reached, cannot continue the search.");
//       break;
//     }

//     current_node.GetNeighbors(&neighbors);

//     if (neighbors.has_forward)
//     {
//       it = std::find_if(closed_list.begin(), closed_list.end(), [&](const AStarNode &node)
//                         { return node.state == neighbors.forward; });
//       if (it != closed_list.end())
//       {
//         continue;
//       }

//       double cost = current_node.cost + neighbors.cost_forward;
//       AStarNode neighborNode{
//           Action::MOVE_FORWARD,
//           closed_list_size,
//           neighbors.forward,
//           cost};
//       open_list.push(neighborNode);
//     }

//     if (neighbors.has_backward)
//     {
//       it = std::find_if(closed_list.begin(), closed_list.end(), [&](const AStarNode &node)
//                         { return node.state == neighbors.backward; });
//       if (it != closed_list.end())
//       {
//         continue;
//       }

//       double cost = current_node.cost + neighbors.cost_backward;
//       AStarNode neighborNode{
//           Action::MOVE_BACKWARD,
//           closed_list_size,
//           neighbors.backward,
//           cost};
//       open_list.push(neighborNode);
//     }

//     it = std::find_if(closed_list.begin(), closed_list.end(), [&](const AStarNode &node)
//                       { return node.state == neighbors.turn_left; });
//     if (it == closed_list.end())
//     {
//       double cost = current_node.cost + neighbors.cost_turn_left;
//       AStarNode neighborNode{
//           Action::TURN_LEFT,
//           closed_list_size,
//           neighbors.turn_left,
//           cost};
//       open_list.push(neighborNode);
//     }

//     it = std::find_if(closed_list.begin(), closed_list.end(), [&](const AStarNode &node)
//                       { return node.state == neighbors.turn_right; });
//     if (it == closed_list.end())
//     {
//       double cost = current_node.cost + neighbors.cost_turn_right;
//       AStarNode neighborNode{
//           Action::TURN_RIGHT,
//           closed_list_size,
//           neighbors.turn_right,
//           cost};
//       open_list.push(neighborNode);
//     }
//   }

//   return Action::UNDEFINED; // No solution found.
// }
