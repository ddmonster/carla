#include "AttentionModel.h"
#include "DReyeVRUtils.h" // ReadConfigVariable
#include "EgoVehicle.h"   // AEgoVehicle

namespace SituationalAwareness
{

AttentionModel::AttentionModel()
{
    // initialize metaparams from the config file
    ReadConfigValue("AttentionModel", "ComprehendedThreshold", ComprehendedThresholdSeconds);
    ReadConfigValue("AttentionModel", "LookAwayThreshold", LookAwayThresholdSeconds);
    ReadConfigValue("AttentionModel", "DetectedThresholdLifetime", DetectedThresholdLifetimeSeconds);
    ReadConfigValue("AttentionModel", "ComprehendedThresholdLifetime", ComprehendedThresholdLifetimeSeconds);

    ReadConfigValue("AttentionModel", "Glow", Glow);
    ReadConfigValue("AttentionModel", "UndetectedState", UndetectedCol);
    ReadConfigValue("AttentionModel", "DetectedState", DetectedCol);
    ReadConfigValue("AttentionModel", "ComprehendedState", ComprehendedCol);
}

void AttentionModel::Evaluate(const float DeltaSeconds, const float CurrentTime, ADReyeVRCustomActor *Overlay,
                              AActor *Actor, AEgoVehicle *EgoVehiclePtr)
{
    bool bHasAttention = false;
    ensure(EgoVehiclePtr != nullptr);
    ensure(Actor != nullptr);
    ensure(Overlay != nullptr);

    struct SituationalAwareness::ElementSA Awareness;

    // see if we have seen this actor before
    if (SceneElements.find(Actor) != SceneElements.end())
    {
        Awareness = SceneElements[Actor]; // have seen this actor before
    }

    // check if we are looking at it now
    if (WithinROICondition(Overlay, Actor, EgoVehiclePtr))
    {
        // we have a hit!

        // should be at least detected
        if (Awareness.State == AttentionState::UNDETECTED)
        {
            Awareness.State = AttentionState::DETECTED; // Awareness for this actor is at least detected
            Awareness.Bday = CurrentTime;               // when we first started looking at it
        }

        // now we know the awareness state is not-undetected
        ensure(Awareness.State == AttentionState::DETECTED || Awareness.State == AttentionState::COMPREHENDED);

        // we want "time looked at" to be somewhat contiguous, so we penalize looking "away" for some time
        if (CurrentTime - Awareness.LastLookTime > LookAwayThresholdSeconds)
        {
            Awareness.TimeLookedAt = 0.f; // reset time looked at if it has been alive too long
        }
        Awareness.LastLookTime = CurrentTime;

        // check if been looking at it long enough (comprehended)
        Awareness.TimeLookedAt += DeltaSeconds; // lookeed at it on this frame (which lasted this long)
        if (Awareness.TimeLookedAt >= ComprehendedThresholdSeconds)
        {
            // keep the comprehended element "alive" for longer if they are still looked at
            Awareness.State = AttentionState::COMPREHENDED;
            Awareness.Bday = CurrentTime; // reset birthday so this awareness can live its lifetime as COMPREHENDED
        }
    }

    // update alive-ness of the current state
    {
        const float CurrentAge = CurrentTime - Awareness.Bday;
        if (Awareness.State == AttentionState::DETECTED && CurrentAge > DetectedThresholdLifetimeSeconds)
        {
            Awareness.State = AttentionState::UNDETECTED; // transition to undetected
            Awareness.Bday = 0.f;                         // reset birthday to "unborn"
            Awareness.TimeLookedAt = 0.f;                 // haven't looked at it since reset
        }
        else if (Awareness.State == AttentionState::COMPREHENDED && CurrentAge > ComprehendedThresholdLifetimeSeconds)
        {
            Awareness.State = AttentionState::DETECTED; // transition to detected
            Awareness.Bday = CurrentTime;               // so the state can be in "Detected" for its remaining lifetime
            Awareness.TimeLookedAt = 0.f;               // haven't looked at it since reset
        }
    }

    // update actor AR overlay
    {
        FLinearColor Col;
        switch (Awareness.State)
        {
        case (AttentionState::UNDETECTED): {
            Col = UndetectedCol;
            break;
        }
        case (AttentionState::DETECTED): {
            Col = DetectedCol;
            break;
        }
        case (AttentionState::COMPREHENDED):
        default: {
            Col = ComprehendedCol;
            break;
        }
        }
        float Opacity = Col.A; // between 0 and 1
        Col.A = 1.0f;          // color is just RGB (no A)
        Overlay->MaterialParams.BaseColor = Col;
        Overlay->MaterialParams.Emissive = Glow * Col;
        Overlay->MaterialParams.Opacity = Opacity;
    }

    // keep track of this actor's awareness for future calls
    SceneElements[Actor] = Awareness;
}

bool AttentionModel::WithinROICondition(const ADReyeVRCustomActor *Overlay, const AActor *Actor,
                                        const AEgoVehicle *EgoVehiclePtr) const
{
    // high-precision ray trace (no threshold)
    // const FString &EyeFocusActorName = EgoVehiclePtr->GetSensor()->GetData()->GetFocusActorName();
    // return (EyeFocusActorName.Equals(Actor->GetName()));

    float TraceRadius = 30.0f; // radius in cm
    FHitResult Hit;
    bool bDidHit = EgoVehiclePtr->GetSensor()->ComputeGazeTrace(Hit, ECC_Visibility, TraceRadius);
    return bDidHit && (Hit.Actor == Actor);
}

} // namespace SituationalAwareness