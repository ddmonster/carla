#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor
#include "Carla/Game/CarlaEpisode.h"        // UCarlaEpisode
#include "Tickable.h"                       // FTickableGameObject
#include <unordered_map>                    // std::unordered_map

class FOverlayManager : public FTickableGameObject
{
  public:
    FOverlayManager(class AActor *EgoVehicle)
    {
        EgoVehiclePtr = EgoVehicle;
    }
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(ArrowOverlayManager, STATGROUP_Tickables);
    }
    void SetWorld(UWorld *World);
    void Destroy();
    enum ActorOverlayType : uint8_t
    {
        UP = 0,
        DOWN,
        LEFT,
        RIGHT,
        // add others here
        SIZE, // used as metadata to get the number of these elements
    };

  private:
    struct ActorOverlayData
    {
        enum ActorOverlayType OverlayType;
        class ADReyeVRCustomActor *Overlay;
        // add other data here if needed
        static ActorOverlayData New(UWorld *World, FCarlaActor *Actor)
        {
            ensure(Actor != nullptr);
            ensure(World != nullptr);
            ActorOverlayData Data;
            int Type = rand() % ActorOverlayType::SIZE; /// TODO: use some better policy for overlay
            Data.OverlayType = static_cast<ActorOverlayType>(Type);
            auto Name = "ActorGTOverlay" + Actor->GetActor()->GetName();
            Data.Overlay = ADReyeVRCustomActor::CreateNew(SM_CONE, MAT_TRANSLUCENT, World, Name);
            UE_LOG(LogTemp, Log, TEXT("Creating overlay for %s as %d"), *Actor->GetActor()->GetName(), Type);
            return Data;
        }
    };

    FRotator OverlayTypeRotationCone(enum ActorOverlayType Type);
    class UWorld *World = nullptr;
    class UCarlaEpisode *Episode = nullptr;
    std::unordered_map<FCarlaActor *, ActorOverlayData> OverlayMapping;
    class AActor *EgoVehiclePtr = nullptr;
};