#include "gladiator.h"

#include "strategy.hpp"

#define FREE_MODE true

Gladiator *gladiator;
Strategy *strategy = nullptr;

void reset();

void setup()
{
  gladiator = new Gladiator();
  strategy = new Strategy(gladiator);

  if (FREE_MODE)
  {
    gladiator->game->enableFreeMode(RemoteMode::OFF);
  }

  gladiator->game->onReset(&reset);
}

void reset()
{
  if (strategy != nullptr)
  {
    delete strategy;
  }
  strategy = new Strategy(gladiator);
}

void loop()
{
  if (gladiator->game->isStarted() || FREE_MODE)
  {
    strategy->Update();
  }
}