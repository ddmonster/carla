#include "CustomActors.h"

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

    AssignSM("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'");

    AssignMat("MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::SPHERE);
    Internals.Other = "TestSphere";
}

ACross *ACross::RequestNewActor(UWorld *World, const DReyeVR::CustomActorData &Init)
{
    check(World != nullptr);
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACross *Actor = World->SpawnActor<ACross>(Init.Location, Init.Rotation, SpawnInfo);
    Actor->Initialize(Init);
    return Actor;
}

ACross::ACross(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    ActorMesh->SetupAttachment(this->GetRootComponent());

    AssignSM("StaticMesh'/Engine/BasicShapes/Cube.Cube'");

    AssignMat("MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::CROSS);
    Internals.Other = "TestCross";
}
