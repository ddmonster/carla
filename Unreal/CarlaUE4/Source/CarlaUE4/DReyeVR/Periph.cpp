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

    // generate stimuli every TimeBetweenFlash second chunks, and log that time
    /// TODO: all these magic numbers need to be parameterized
    if (TimeSinceLastFlash < MaxTimeBetweenFlash + FlashDuration)
    {
        if (TimeSinceLastFlash == 0.f)
        {
            // update time for the next periph trigger
            NextPeriphTrigger = FMath::RandRange(MinTimeBetweenFlash, MaxTimeBetweenFlash);

            // generate random position for the next periph target
            float RandYaw = FMath::RandRange(PeriphYawBounds.X, PeriphYawBounds.Y);
            float RandPitch = FMath::RandRange(PeriphPitchBounds.X, PeriphPitchBounds.Y);
            float Roll = 0.f;
            PeriphRotator = FRotator(RandPitch, RandYaw, Roll);
        }
        else if (LastPeriphTick <= NextPeriphTrigger && TimeSinceLastFlash > NextPeriphTrigger)
        {
            // turn on periph target
            ensure(PeriphTarget == nullptr);
            PeriphTarget = ABall::RequestNewActor(World, "PeriphTarget");
            UE_LOG(LogTemp, Log, TEXT("Periph Target On @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
        }
        else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration &&
                 TimeSinceLastFlash > NextPeriphTrigger + FlashDuration)
        {
            // turn off periph target
            ensure(PeriphTarget != nullptr);
            PeriphTarget->RequestDestroy();
            PeriphTarget = nullptr;
            UE_LOG(LogTemp, Log, TEXT("Periph Target Off @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
        }
        LastPeriphTick = TimeSinceLastFlash;
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
    const FVector TopLeft = HeadDirection.RotateVector(
        (PeriphRotationOffset + FRotator(PeriphPitchBounds.X, PeriphYawBounds.X, 0.f)).Vector());
    const FVector TopRight = HeadDirection.RotateVector(
        (PeriphRotationOffset + FRotator(PeriphPitchBounds.X, PeriphYawBounds.Y, 0.f)).Vector());
    const FVector BotLeft = HeadDirection.RotateVector(
        (PeriphRotationOffset + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.X, 0.f)).Vector());
    const FVector BotRight = HeadDirection.RotateVector(
        (PeriphRotationOffset + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.Y, 0.f)).Vector());
    DrawDebugSphere(World, HeadPos + TopLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, HeadPos + TopRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, HeadPos + BotLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, HeadPos + BotRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
#endif

    if (PeriphTarget != nullptr)
    {
        const FVector PeriphFinal =
            GetData()->GetCameraRotationAbs().RotateVector((PeriphRotationOffset + PeriphRotator).Vector());
        PeriphTarget->SetActorLocation(Camera->GetComponentLocation() + PeriphFinal * TargetRenderDistance * 100.f);
        PeriphTarget->SetActorScale3D(PeriphTargetRadius * FVector::OneVector);
    }
}