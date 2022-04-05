#include "CustomActors.h"
#include "DReyeVRUtils.h" // Read

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
}

APeriphTarget::APeriphTarget(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'");

    // create emissive red texture
    check(ActorMesh != nullptr);
    UMaterialInstanceDynamic *DynMat = ActorMesh->CreateAndSetMaterialInstanceDynamic(0);
    float Emission = 0.f;
    ReadConfigValue("PeripheralTarget", "Emission", Emission);
    DynMat->SetScalarParameterValue("Emission", Emission);
    DynMat->SetVectorParameterValue("Color", FLinearColor::Red); // bright red
    ActorMesh->SetMaterial(0, DynMat);

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::PERIPH_TARGET);
}
