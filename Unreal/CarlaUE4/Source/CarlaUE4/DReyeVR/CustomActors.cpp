#include "CustomActors.h"
#include "DReyeVRUtils.h" // ReadConfigValue

ASphere::ASphere(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'");

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::SPHERE);
}

ACube::ACube(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Engine/BasicShapes/Cube.Cube'");

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::CUBE);
}

ACone::ACone(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Engine/BasicShapes/Cone.Cone'");

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::CONE);
}

ACross::ACross(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Game/DReyeVR/Custom/Periph/SM_FixationCross.SM_FixationCross'");
    NumMaterials = 2;

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::CROSS);
}

AArrow::AArrow(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Engine/BasicShapes/Cube.Cube'");

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::ARROW);
}
