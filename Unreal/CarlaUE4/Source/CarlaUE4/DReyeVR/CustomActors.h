#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor

#include "CustomActors.generated.h"

/// TODO: template partial specialization metaprogramming
UCLASS()
class ABall : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ABall(const FObjectInitializer &ObjectInitializer);

    static ABall *RequestNewActor(UWorld *World, const FString &Name);
};

UCLASS()
class ACross : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ACross(const FObjectInitializer &ObjectInitializer);

    static ACross *RequestNewActor(UWorld *World, const FString &Name);
};