#include "Carla/Actor/ActorData.h"
#include "Carla/Actor/ActorInfo.h"
#include "Carla/Traffic/TrafficLightState.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerController.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"

#include "Carla/Game/CarlaEpisode.h"

#include "CoreMinimal.h"
#include "Tickable.h"

class FOverlayManager : public FTickableGameObject
{
  public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(ArrowOverlayManager, STATGROUP_Tickables);
    }
    int EgoVehicleID;
    FOverlayManager(int EgoVehicleID);
    void SetEpisode(UCarlaEpisode *Episode);
    void SetWorld(UWorld *World);
    void Destroy();

  private:
    class UWorld *World = nullptr;
    class UCarlaEpisode *Episode = nullptr;
};