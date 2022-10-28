#pragma once

#include "Carla/Actor/CarlaActor.h"        // FCarlaActor
#include "Carla/Actor/CarlaActorFactory.h" // ACarlaActorFactory
#include <vector>

class DummyWalkers
{
  public:
    DummyWalkers();
    void Setup(UWorld *World);
    void Tick(UWorld *World, const float DeltaSeconds);

  private:
    std::vector<FCarlaActor *> Walkers;
    size_t NumWalkers;
    int Seed;
};