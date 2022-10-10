#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor

class AEgoVehicle; // to not need to #include EgoVehicle.h which #includes LevelBP which #includes this

// Measuring Driver Situation Awareness Using Region-of-Interest Prediction and Eye Tracking
namespace SituationalAwareness
{

enum AttentionState : uint8_t
{
    COMPREHENDED = 0,
    DETECTED,
    UNDETECTED,
};

struct ElementSA
{
    ElementSA() = default;
    float Bday = 0.f;         // "birthday" of when this element was first looked upon
    float TimeLookedAt = 0.f; // how long (and counting) has this element been looked at for
    float LastLookTime = 0.f; // when was the last time this element was looked at
    enum AttentionState State = AttentionState::UNDETECTED;
};

class AttentionModel
{
  public:
    AttentionModel() = default;

    void Evaluate(const float DeltaSeconds, float CurrentTime, ADReyeVRCustomActor *Overlay, AActor *Actor,
                  AEgoVehicle *EgoVehiclePtr);

  private:
    bool WithinROICondition(const ADReyeVRCustomActor *Overlay, const AActor *Actor,
                            const AEgoVehicle *EgoVehiclePtr) const;

    std::unordered_map<AActor *, ElementSA> SceneElements;

    // metaparameters
    const float ComprehendedThresholdSeconds = 0.5f; // if dwelled for this time (s) => comprehended
    const float LookAwayThresholdSeconds = 0.2;

    // number of seconds an actor can stay in this "state" before downgrading
    const float DetectedThresholdLifetimeSeconds = 1.f;
    const float ComprehendedThresholdLifetimeSeconds = 5.f;
};

} // namespace SituationalAwareness