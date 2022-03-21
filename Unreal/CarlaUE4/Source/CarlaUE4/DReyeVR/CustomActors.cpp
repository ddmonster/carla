#include "CustomActors.h"
#include "UObject/ConstructorHelpers.h" // ConstructorHelpers

ABall *ABall::RequestNewActor(UWorld *World, const DReyeVR::CustomActorData &Init)
{
    check(World != nullptr);
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ABall *Actor = World->SpawnActor<ABall>(Init.Location, Init.Rotation, SpawnInfo);
    Actor->Initialize(Init);
    return Actor;
}

ABall::ABall(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    ActorMesh->SetupAttachment(this->GetRootComponent());

    const FString MeshAssetPath = "StaticMesh'/Engine/BasicShapes/Sphere.Sphere'";
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(*MeshAssetPath);
    if (MeshAsset.Succeeded())
        ActorMesh->SetStaticMesh(MeshAsset.Object);
    else
        UE_LOG(LogTemp, Error, TEXT("Unable to access mesh asset: %s"), *MeshAssetPath)

    // const FString MaterialAssetPath = "Material'/Game/Carla/Static/GenericMaterials/M_Red.M_Red'";
    const FString MaterialAssetPath =
        "MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'";
    static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialAsset(*MaterialAssetPath);
    if (MaterialAsset.Succeeded())
        ActorMesh->SetMaterial(0, MaterialAsset.Object);
    else
        UE_LOG(LogTemp, Error, TEXT("Unable to access material asset: %s"), *MaterialAssetPath)

    // finalizing construction
    this->SetActorEnableCollision(false); // currently only "HUD-like" objects

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::SPHERE);
    Internals.Other = "Test";
}
