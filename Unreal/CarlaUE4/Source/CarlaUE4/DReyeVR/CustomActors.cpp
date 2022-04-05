#include "CustomActors.h"

ABall::ABall(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'");

    AssignMat(0, "MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::SPHERE);
    Internals.Other = "";
}

ACross::ACross(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Game/DReyeVR/Custom/SMCross.SMCross'");

    // use default material
    AssignMat(0, "MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");
    AssignMat(1, "MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::CROSS);
    Internals.Other = "";
}
