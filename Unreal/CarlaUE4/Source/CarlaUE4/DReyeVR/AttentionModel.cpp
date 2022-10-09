#include "AttentionModel.h"

void AttentionModel::Evaluate(float CurrentTime, ADReyeVRCustomActor *Overlay, AActor *Actor,
                              AEgoVehicle *EgoVehiclePtr)
{
    bool bHasAttention = false;
    ensure(EgoVehiclePtr != nullptr);
    ensure(Actor != nullptr);
    ensure(Overlay != nullptr);

    const FString &EyeFocusActorName = EgoVehiclePtr->GetSensor()->GetData()->GetFocusActorName();

    if (EyeFocusActorName.Equals(Actor->GetName()))
    {
        // we have an (ACTOR) hit!
        if (ActorHitCount == 0)
        {
            // only start the count on the first hit
            StartTimeHit = CurrentTime;
        }
        ActorHitCount += 1;
        UE_LOG(LogTemp, Log, TEXT("Hit count: %d time: %.3f"), ActorHitCount, CurrentTime);

        // deactivate the overlay
        if (ActorHitCount > MaxHitCountThreshold) // more than 10 hits in the time threshold
        {
            Actor->Tags.Remove(OverlayTag);
            Overlay->Deactivate();
        }
    }

    if (CurrentTime - StartTimeHit > MaxHitCountThresholdSeconds)
    {
        // reset count
        ActorHitCount = 0;
    }
}