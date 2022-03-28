#include "EgoSensor.h"

void AEgoSensor::TickPeriphTarget(float DeltaTime)
{
    if (bUseFixedCrosshair)
    {
        if (Crosshair == nullptr)
            Crosshair = ACross::RequestNewActor(World, "PeriphCrosshair");
        const FVector CrosshairVector = GetData()->GetCameraRotationAbs().RotateVector(FVector::ForwardVector);
        Crosshair->SetActorLocation(Camera->GetComponentLocation() + CrosshairVector * TargetRenderDistance * 100.f);
        Crosshair->SetActorRotation(Camera->GetComponentRotation());
        Crosshair->SetActorScale3D(0.1f * FVector::OneVector);
    }

    if (!bUsePeriphTarget)
        return;

    if (IsReplaying() && bUsingLegacyPeriphFile)
    {
        // replay of legacy periph target is handled in UpdateData
        if (PeriphTarget != nullptr)
            PeriphTarget->RequestDestroy();
        return;
    }

    const FVector2D YawBounds(-40, 40);   // (degrees)
    const FVector2D PitchBounds(-20, 20); // (degrees)

    // generate stimuli every TimeBetweenFlash second chunks, and log that time
    /// TODO: all these magic numbers need to be parameterized
    if (TimeSinceLastFlash < MaxTimeBetweenFlash)
    {
        if (TimeSinceLastFlash == 0.f)
        {
            // update time for the next periph trigger
            NextPeriphTrigger = FMath::RandRange(MinTimeBetweenFlash, MaxTimeBetweenFlash);

            // generate random position for the next periph target
            float RandYaw = FMath::RandRange(YawBounds.X, YawBounds.Y);
            float RandPitch = FMath::RandRange(PitchBounds.X, PitchBounds.Y);
            float Roll = 0.f;
            PeriphVector = GetData()->GetCameraRotationAbs().RotateVector(FRotator(RandPitch, RandYaw, Roll).Vector());
        }
        else if (FMath::IsNearlyEqual(TimeSinceLastFlash, NextPeriphTrigger, 0.05f))
        {
            // turn on periph target
            if (PeriphTarget == nullptr)
            {
                PeriphTarget = ABall::RequestNewActor(World, "PeriphTarget");
                UE_LOG(LogTemp, Log, TEXT("Periph Target On @ %f"), TimeSinceLastFlash);
            }
        }
        else if (FMath::IsNearlyEqual(TimeSinceLastFlash, NextPeriphTrigger + FlashDuration, 0.05f))
        {
            // turn off periph target
            if (PeriphTarget)
            {
                PeriphTarget->RequestDestroy();
                PeriphTarget = nullptr;
                UE_LOG(LogTemp, Log, TEXT("Periph Target Off @ %f"), TimeSinceLastFlash);
            }
        }
        TimeSinceLastFlash += DeltaTime;
    }
    else
    {
        // reset the periph flashing
        TimeSinceLastFlash = 0.f;
    }

// Draw debug border markers
#if WITH_EDITOR
    const FRotator &HeadDirection = GetData()->GetCameraRotationAbs();
    const FVector &HeadPos = GetData()->GetCameraLocationAbs();
    const FVector TopLeft = HeadDirection.RotateVector(FRotator(PitchBounds.X, YawBounds.X, 0.f).Vector());
    const FVector TopRight = HeadDirection.RotateVector(FRotator(PitchBounds.X, YawBounds.Y, 0.f).Vector());
    const FVector BotLeft = HeadDirection.RotateVector(FRotator(PitchBounds.Y, YawBounds.X, 0.f).Vector());
    const FVector BotRight = HeadDirection.RotateVector(FRotator(PitchBounds.Y, YawBounds.Y, 0.f).Vector());
    DrawDebugSphere(World, HeadPos + TopLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, HeadPos + TopRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, HeadPos + BotLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, HeadPos + BotRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
#endif

    if (PeriphTarget != nullptr)
    {
        PeriphTarget->SetActorLocation(Camera->GetComponentLocation() + PeriphVector * TargetRenderDistance * 100.f);
        PeriphTarget->SetActorScale3D(0.1f * FVector::OneVector);
    }
}