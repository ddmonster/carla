#include "Periph.h"
#include "DReyeVRUtils.h" // ReadConfigValue

PeriphSystem::PeriphSystem()
{
    ReadConfigVariables();
}

void PeriphSystem::ReadConfigVariables()
{
    // peripheral target
    ReadConfigValue("PeripheralTarget", "EnablePeriphTarget", bUsePeriphTarget);
    ReadConfigValue("PeripheralTarget", "YawBounds", PeriphYawBounds);
    ReadConfigValue("PeripheralTarget", "PitchBounds", PeriphPitchBounds);
    ReadConfigValue("PeripheralTarget", "RotationOffset", PeriphRotationOffset);
    ReadConfigValue("PeripheralTarget", "MaxTimeBetweenFlashSec", MaxTimeBetweenFlash);
    ReadConfigValue("PeripheralTarget", "MinTimeBetweenFlashSec", MinTimeBetweenFlash);
    ReadConfigValue("PeripheralTarget", "FlashDurationSec", FlashDuration);
    ReadConfigValue("PeripheralTarget", "PeriphTargetSize", PeriphTargetSize);
    // fixation cross params
    ReadConfigValue("PeripheralTarget", "EnableFixedCross", bUseFixedCross);
    ReadConfigValue("PeripheralTarget", "FixationCrossSize", FixationCrossSize);
    // general
    ReadConfigValue("PeripheralTarget", "TargetRenderDistanceM", TargetRenderDistance);
}

void PeriphSystem::Initialize(class UWorld *WorldIn)
{
    World = WorldIn;
    check(World != nullptr);
    if (bUseFixedCross)
    {
        Cross = ADReyeVRCustomActor::CreateNew(SM_CROSS, World, PeriphFixationName, 2);
        Cross->SetActorScale3D(FixationCrossSize * FVector::OneVector);
        Cross->AssignMat(ADReyeVRCustomActor::OpaqueMaterial);
        check(Cross != nullptr);
    }
    if (bUsePeriphTarget)
    {
        PeriphTarget = ADReyeVRCustomActor::CreateNew(SM_SPHERE, World, PeriphTargetName);
        PeriphTarget->SetActorScale3D(PeriphTargetSize * FVector::OneVector);
        PeriphTarget->AssignMat(ADReyeVRCustomActor::OpaqueMaterial);
        float Emissive;
        ReadConfigValue("PeripheralTarget", "EmissionFactor", Emissive);
        PeriphTarget->MaterialParams.Emissive = Emissive * FLinearColor::Red;
        check(PeriphTarget != nullptr);
    }
}

void PeriphSystem::Tick(float DeltaTime, bool bIsReplaying, bool bInCleanRoomExperiment, const UCameraComponent *Camera)
{
    // if replaying, don't tick periph system
    if (bIsReplaying || World == nullptr)
    {
        // replay of legacy periph target is handled in UpdateData
        if (PeriphTarget != nullptr && PeriphTarget->IsActive())
            PeriphTarget->Deactivate();
        if (Cross != nullptr && Cross->IsActive())
            Cross->Deactivate();
        return;
    }

    const FVector &CameraLoc = Camera->GetComponentLocation();
    const FRotator &CameraRot = Camera->GetComponentRotation();
    if (bUseFixedCross)
    {
        if (bInCleanRoomExperiment)
        {
            const FVector CrossVector = CameraRot.RotateVector(FVector::ForwardVector);
            Cross->SetActorLocation(CameraLoc + CrossVector * TargetRenderDistance * 100.f);
            Cross->SetActorRotation(CameraRot);
            Cross->Activate();
        }
        else if (Cross->IsActive())
        {
            Cross->Deactivate();
        }
    }

    if (bUsePeriphTarget)
    {
        // generate stimuli every TimeBetweenFlash second chunks, and log that time
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
                PeriphTarget->Activate();
                UE_LOG(LogTemp, Log, TEXT("Periph Target On \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
            }
            else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration &&
                     TimeSinceLastFlash > NextPeriphTrigger + FlashDuration)
            {
                // turn off periph target
                PeriphTarget->Deactivate();
                UE_LOG(LogTemp, Log, TEXT("Periph Target Off \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
            }
            LastPeriphTick = TimeSinceLastFlash;
            TimeSinceLastFlash += DeltaTime;
        }
        else
        {
            // reset the periph flashing
            TimeSinceLastFlash = 0.f;
        }
        if (PeriphTarget->IsActive())
        {
            const FVector PeriphFinal = CameraRot.RotateVector((PeriphRotationOffset + PeriphRotator).Vector());
            PeriphTarget->SetActorLocation(CameraLoc + PeriphFinal * TargetRenderDistance * 100.f);
        }

// Draw debug border markers
#if WITH_EDITOR
        const FVector TopLeft = CameraRot.RotateVector(
            (PeriphRotationOffset + FRotator(PeriphPitchBounds.X, PeriphYawBounds.X, 0.f)).Vector());
        const FVector TopRight = CameraRot.RotateVector(
            (PeriphRotationOffset + FRotator(PeriphPitchBounds.X, PeriphYawBounds.Y, 0.f)).Vector());
        const FVector BotLeft = CameraRot.RotateVector(
            (PeriphRotationOffset + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.X, 0.f)).Vector());
        const FVector BotRight = CameraRot.RotateVector(
            (PeriphRotationOffset + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.Y, 0.f)).Vector());
        DrawDebugSphere(World, CameraLoc + TopLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
        DrawDebugSphere(World, CameraLoc + TopRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
        DrawDebugSphere(World, CameraLoc + BotLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
        DrawDebugSphere(World, CameraLoc + BotRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
#endif
    }
}