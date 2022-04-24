#pragma once

#include "Carla/Sensor/DReyeVRData.h" // DReyeVR namespace
#include "GameFramework/Actor.h"      // AActor

#include <unordered_map> // std::unordered_map
#include <utility>       // std::pair
#include <vector>        // std::vector

#include "DReyeVRCustomActor.generated.h"

// define some paths to common custom actor types
#define SM_SPHERE "StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"
#define SM_CUBE "StaticMesh'/Engine/BasicShapes/Cube.Cube'"
#define SM_CONE "StaticMesh'/Engine/BasicShapes/Cone.Cone'"
#define SM_CROSS "StaticMesh'/Game/DReyeVR/Custom/Periph/SM_FixationCross.SM_FixationCross'"
/// TODO: make Arrow mesh
#define SM_ARROW "StaticMesh'/Engine/BasicShapes/Cube.Cube'"

UCLASS()
class CARLA_API ADReyeVRCustomActor : public AActor // abstract class
{
    GENERATED_BODY()

  public:
    /// factory function to create a new instance of a given type
    static ADReyeVRCustomActor *CreateNew(const FString &SM_Path, UWorld *World, const FString &Name,
                                          const int KnownNumMaterials = 1);

    virtual void Tick(float DeltaSeconds) override;

    void Activate();
    void Deactivate();
    bool IsActive() const
    {
        return bIsActive;
    }

    const static FString OpaqueMaterial, TranslucentMaterial;

    void Initialize(const FString &Name);

    void SetInternals(const DReyeVR::CustomActorData &In);

    const DReyeVR::CustomActorData &GetInternals() const;

    static std::unordered_map<std::string, class ADReyeVRCustomActor *> ActiveCustomActors;

    // function to dynamically change the material params of the object at runtime
    void AssignMat(const FString &Path);
    struct DReyeVR::CustomActorData::MaterialParamsStruct MaterialParams;

  private:
    ADReyeVRCustomActor(const FObjectInitializer &ObjectInitializer);
    void BeginPlay() override;
    void BeginDestroy() override;
    bool bIsActive = false; // initially deactivated

    bool AssignSM(const FString &Path, UWorld *World);

    class DReyeVR::CustomActorData Internals;

    UPROPERTY(EditAnywhere, Category = "Mesh")
    class UStaticMeshComponent *ActorMesh = nullptr;
    static int AllMeshCount;

    UPROPERTY(EditAnywhere, Category = "Materials")
    class UMaterialInstanceDynamic *DynamicMat = nullptr;
    int NumMaterials = 1; // change this to apply the dynamic material to more than 1 materials
};