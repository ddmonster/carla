#include "OverlayManager.h"
#include "Carla/Actor/ActorRegistry.h" // FActorRegistry
#include "Carla/Actor/CarlaActor.h"    // FCarlaActor
#include "Carla/Game/CarlaStatics.h"   // GetCurrentEpisodes
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

void FOverlayManager::Tick(float DeltaTime)
{
#if 0 // fails if registry is not perfectly aligned with world (fails on --reloadWorld)
    if (!Episode || !EgoVehiclePtr)
        return;

    const FActorRegistry &Registry = Episode->GetActorRegistry();
    for (auto It = Registry.begin(); It != Registry.end(); ++It)
    {
        FCarlaActor *Actor = It.Value().Get();
        if (Actor == nullptr || Actor->GetActor() == EgoVehiclePtr)
            continue; // skip the EgoVehicle

        FCarlaActor::ActorType type = Actor->GetActorType();
        if (type == FCarlaActor::ActorType::Vehicle || type == FCarlaActor::ActorType::Walker)
        {
            if (Actor->GetActor()->WasRecentlyRendered(DeltaTime))
            {
                if (OverlayMapping.find(Actor) == OverlayMapping.end())
                {
                    // assign some overlay to this actor
                    OverlayMapping[Actor] = ActorOverlayData::New(World, Actor);
                }
                ensure(OverlayMapping.find(Actor) != OverlayMapping.end());

                // FVector Pos = Actor->GetActorGlobalLocation();
                FVector BBox_Offset, BBox_Extent;
                Actor->GetActor()->GetActorBounds(true, BBox_Offset, BBox_Extent, false);
                float Height = 2 * BBox_Extent.Z; // extent is only half the "volume" (extension from center)
                // 1m (100 cm) from the height of the actor
                FVector Pos = Actor->GetActor()->GetActorLocation() + (Height + 100.f) * FVector::UpVector;

                /// TODO: make rotation relative to EgoVehicle
                ensure(OverlayMapping[Actor].Overlay != nullptr);
                auto &Overlay = *(OverlayMapping[Actor].Overlay);
                { // enable overlay and move correctly
                    Overlay.Activate();
                    auto Col = FLinearColor::Red;
                    Overlay.MaterialParams.BaseColor = Col;
                    Overlay.MaterialParams.Emissive = 0.5 * Col;
                    Overlay.MaterialParams.Opacity = 0.5;
                    Overlay.SetActorScale3D(0.5 * FVector::OneVector);
                    Overlay.SetActorLocation(Pos);
                    FRotator Rotation = OverlayTypeRotationCone(OverlayMapping[Actor].OverlayType);
                    Overlay.SetActorRotation(Rotation);
                }
            }
        }
    }
#endif
}

void FOverlayManager::SetWorld(UWorld *ThisWorld)
{
    ensure(ThisWorld != nullptr);
    World = ThisWorld;
    Episode = UCarlaStatics::GetCurrentEpisode(World);
    ensure(Episode != nullptr);
}

void FOverlayManager::Destroy()
{
}

FRotator FOverlayManager::OverlayTypeRotationCone(enum FOverlayManager::ActorOverlayType OverlayType)
{
    // FRotator{Pitch, Yaw, Roll}
    // assuming the Overlay is a cone (default pointing up)
    if (OverlayType == FOverlayManager::ActorOverlayType::UP)
    {
        return FRotator{0, 0, 0};
    }
    else if (OverlayType == FOverlayManager::ActorOverlayType::DOWN)
    {
        return FRotator{0, 180, 0};
    }
    else if (OverlayType == FOverlayManager::ActorOverlayType::LEFT)
    {
        return FRotator{90, 90, 0};
    }
    else if (OverlayType == FOverlayManager::ActorOverlayType::RIGHT)
    {
        return FRotator{90, -90, 0};
    }
    return FRotator::ZeroRotator;
}