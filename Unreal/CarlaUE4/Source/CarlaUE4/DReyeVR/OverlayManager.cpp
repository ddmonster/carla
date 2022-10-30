#include "OverlayManager.h"
#include "Carla/Actor/ActorInfo.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Actor/CarlaActor.h"
#include "Carla/Game/CarlaStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "carla/rpc/ActorState.h"
#include "carla/rpc/VehicleLightState.h"

void FOverlayManager::Tick(float DeltaTime)
{
    if (!Episode)
        return;

    const FActorRegistry &Registry = Episode->GetActorRegistry();
    int x = 0;
    for (auto It = Registry.begin(); It != Registry.end(); ++It)
    {
        FCarlaActor *Actor = It.Value().Get();
        if (Actor->GetActorId() == EgoVehicleID)
            continue;

        FCarlaActor::ActorType type = Actor->GetActorType();
        if (type == FCarlaActor::ActorType::Vehicle || type == FCarlaActor::ActorType::Walker)
        {
            if (Actor->GetActor()->WasRecentlyRendered(DeltaTime))
            {
                // FVector Pos = Actor->GetActorGlobalLocation();
                FVector Pos = Actor->GetActor()->GetActorLocation();
                DrawDebugSphere(World, Pos, 25.0f, 12, FColor::Blue);
                ++x;
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Overlay Manager %d"), x);
}

void FOverlayManager::SetWorld(UWorld *ThisWorld)
{
    World = ThisWorld;
}

void FOverlayManager::SetEpisode(UCarlaEpisode *ThisEpisode)
{
    Episode = ThisEpisode;
}

void FOverlayManager::Destroy()
{
}

FOverlayManager::FOverlayManager(int ID)
{
    EgoVehicleID = ID;
}