#pragma once

#include "Carla/Actor/CarlaActor.h"        // FCarlaActor
#include "Carla/Actor/CarlaActorFactory.h" // ACarlaActorFactory
#include <random>                          // std::uniform_real_distribution
#include <vector>

class DummyWalkers
{
  public:
    DummyWalkers();
    void Setup(UWorld *World);
    void Tick(UWorld *World, const float DeltaSeconds);

  private:
    struct WalkerStruct
    {
        FCarlaActor *Walker = nullptr;
        float Speed = 134.f;      // ~3mph (average human walking speed)
        float AngularStep = 45.f; // granularity of angular checks for walkable
    };

    std::vector<WalkerStruct> Walkers;
    std::mt19937 SpawnRNG, DescrRNG, PhysicsRNG;
    std::uniform_real_distribution<float> Unif01;
    int NumWalkers;
    int Seed;
};