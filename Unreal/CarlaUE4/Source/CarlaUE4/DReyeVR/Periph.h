#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor

class PeriphSystem
{
  public:
    PeriphSystem();
    void ReadConfigVariables();

    class ADReyeVRCustomActor *PeriphTarget = nullptr;
    class ADReyeVRCustomActor *Cross = nullptr;

    void Initialize(class UWorld *World);
    void Tick(float DeltaTime, bool bIsReplaying, bool bInCleanRoomExperiment, const UCameraComponent *Camera);
    void LegacyTick(const DReyeVR::LegacyPeriphDataStruct &LegacyData);

  private:
    class UWorld *World = nullptr;

    ///////////////:PERIPH:////////////////
    FRotator PeriphRotator, PeriphRotationOffset;
    FVector2D PeriphYawBounds, PeriphPitchBounds;
    float MaxTimeBetweenFlash;
    float MinTimeBetweenFlash;
    float FlashDuration;
    float PeriphTargetSize, FixationCrossSize;
    float TargetRenderDistance;
    float LastPeriphTick = 0.f;
    float TimeSinceLastFlash = 0.f;
    float NextPeriphTrigger = 0.f;
    bool bUsePeriphTarget = false;
    bool bUseFixedCross = false;
    const FString PeriphTargetName = "PeriphTarget";
    const FString PeriphFixationName = "PeriphCross";
};