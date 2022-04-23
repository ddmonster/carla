#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor

#include "CustomActors.generated.h"

/// NOTE: include this as part of your custom actor instance class!
#define CREATE_REQUEST_FACTORY_FN(T)                                                                                   \
    static T *RequestNewActor(UWorld *World, const FString &Name)                                                      \
    {                                                                                                                  \
        check(World != nullptr);                                                                                       \
        FActorSpawnParameters SpawnInfo;                                                                               \
        SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;                    \
        T *Actor = World->SpawnActor<T>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);                        \
        Actor->Initialize(Name);                                                                                       \
        return Actor;                                                                                                  \
    }

UCLASS()
class ASphere : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ASphere(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(ASphere);
};

UCLASS()
class ACube : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ACube(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(ACube);
};

UCLASS()
class ACone : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ACone(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(ACone);
};

UCLASS()
class ACross : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ACross(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(ACross);
};

UCLASS()
class AArrow : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    AArrow(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(AArrow);
};
