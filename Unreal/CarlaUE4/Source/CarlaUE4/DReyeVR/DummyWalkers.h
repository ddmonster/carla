#pragma once

#include "Carla/Actor/CarlaActor.h"        // FCarlaActor
#include "Carla/Actor/CarlaActorFactory.h" // ACarlaActorFactory
#include <random>                          // std::uniform_real_distribution
#include <unordered_map>                   // std::unordered_map

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
        float Speed = 134.f;         // ~3mph (average human walking speed)
        float AngularStep = 45.f;    // granularity of angular checks for walkable
        float TimeInSamePlace = 0.f; // how long has the actor not moved (to tell is stuck in place)
    };

    WalkerStruct NewWalker(FCarlaActor *Walker)
    {
        struct WalkerStruct WS;
        {
            WS.Walker = Walker;
            float rand_0_1 = Unif01(PhysicsRNG);
            WS.Speed *= (rand_0_1 + 0.5f); // between 50% more/less
        }
        return WS;
    }

    bool FindWalkers(UWorld *World);

    std::unordered_map<AActor *, WalkerStruct> Walkers;
    std::mt19937 SpawnRNG, DescrRNG, PhysicsRNG;
    std::uniform_real_distribution<float> Unif01;
    int NumWalkers;
    int Seed;

    // metaparams
    FName DummyWalkerTag;

    // for accessing all actors (vehicles/walkers only) in the world
    float RefreshActorSearchTick = 0.1f; // tickrate (seconds) for FindWalkers()
    float TimeSinceLastActorRefresh = 0.f;
};