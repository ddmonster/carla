#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor

#include "CustomActors.generated.h"

UCLASS()
class CARLA_API ABall : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ABall(const FObjectInitializer &ObjectInitializer);

    static ABall *RequestNewActor(UWorld *World, const DReyeVR::CustomActorData &Init);
};