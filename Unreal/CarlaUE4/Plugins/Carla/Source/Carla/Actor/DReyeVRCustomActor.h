#pragma once

#include "Carla/Sensor/DReyeVRData.h" // DReyeVR namespace
#include "GameFramework/Actor.h"      // AActor

#include <unordered_map> // std::unordered_map

#include "DReyeVRCustomActor.generated.h"

UCLASS()
class CARLA_API ADReyeVRCustomActor : public AActor // abstract class
{
    GENERATED_BODY()

  public:
    ADReyeVRCustomActor(const FObjectInitializer &ObjectInitializer);

    virtual void Tick(float DeltaSeconds) override;

    static ADReyeVRCustomActor *RequestNewActor(UWorld *World, const DReyeVR::CustomActorData &Init);

    void Initialize(const DReyeVR::CustomActorData &In);

    void SetInternals(const DReyeVR::CustomActorData &In);

    const DReyeVR::CustomActorData &GetInternals() const;

    static std::unordered_map<std::string, class ADReyeVRCustomActor *> ActiveCustomActors;

  protected:
    void BeginPlay();
    void BeginDestroy();

    void AssignSM(const FString &Path);
    void AssignMat(const FString &Path);

    DReyeVR::CustomActorData Internals;

    UPROPERTY(EditAnywhere, Category = "Materials")
    UMaterialInstance *Material = nullptr;

    UPROPERTY(EditAnywhere, Category = "Materials")
    UMaterialInstanceDynamic *DynamicMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Mesh")
    UStaticMeshComponent *ActorMesh = nullptr;
};