#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor
#include "EgoVehicle.h"                     // AEgoVehicle

// Measuring Driver Situation Awareness Using Region-of-Interest Prediction and Eye Tracking
class AttentionModel
{
  public:
    AttentionModel() = default;

    void Evaluate(float CurrentTime, ADReyeVRCustomActor *Overlay, AActor *Actor, AEgoVehicle *EgoVehiclePtr);

  private:
    AActor *CurrentActor = nullptr;
    float StartTimeHit = 0.f;                                  // time when the eye gaze FIRST hit the actor
    size_t ActorHitCount = 0;                                  // number of hits of the actor since the first hit time
    const size_t MaxHitCountThreshold = 10;                    // number of gaze hits to constitute attention
    constexpr static float MaxHitCountThresholdSeconds = 10.f; // number of seconds to reset the "start hit time"
    const FName OverlayTag{"Overlay"};                         // also defined in CarlaActor.cpp::SetActorEnableOverlay
};
