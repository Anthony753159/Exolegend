#include "gladiator.h"

#include "strategy.hpp"
#include "trajectory.hpp"
#include "algos/maze.hpp"
#include "algos/gamestate.hpp"

/* Variables */

/* Contains everything that needs a reset between games */
struct RobotBehaviorComponents
{
  RobotBehaviorComponents(Gladiator *gladiator = nullptr) : strategy(gladiator), trajectory(gladiator) {}

  Strategy strategy;
  Trajectory trajectory;
} components;

Gladiator *gladiator;

/* Used to keep variables between loop() calls */
GameData game_data;

/* Gladiator API functions */

void reset()
{
  components = RobotBehaviorComponents(gladiator);
  game_data = GameData();
}

void setup()
{
  gladiator = new Gladiator();
  components = RobotBehaviorComponents(gladiator);
  game_data = GameData();

  gladiator->game->onReset(&reset);
}

void loop()
{
  if (gladiator->game->isStarted())
  {
    /* Only getting the ID of playing robots once */
    if (game_data.first_loop)
    {
      game_data.first_loop = false;
      game_data.robot_list = gladiator->game->getPlayingRobotsId();
    }

    /* Retrieving robots data from the game master */
    game_data.robot_data = gladiator->robot->getData();
    for (size_t i = 0; i < N_ROBOTS; i++)
    {
      game_data.other_robots_data[i] = gladiator->game->getOtherRobotData(game_data.robot_list.ids[i]);
    }

    /* Always trajectory, which handles robot movement */
    components.trajectory.Update(game_data);

    /* If trajectory says we can begin searching for the next strategy step, we do so */
    if (components.trajectory.ShouldSearchStrategy())
    {
      /* If we already have a valid strategy step next, we avoid recomputing it */
      if (!components.strategy.IsNextMsgValid())
      {
        components.strategy.Update(game_data);
      }
    }

    /* A bit later, the trajectory allows us to apply the selected strategy step */
    if (components.trajectory.ShouldApplyStrategy())
    {
      /* We get the strategy message, consume it (marking it as invalid from now on), and pass it to the trajectory */
      components.trajectory.HandleMessage(components.strategy.GetNextMsg());
      components.strategy.ConsumeMsg();
    }
  }
}