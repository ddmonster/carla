#include "DReyeVRCustomActor.h"
#include "Carla/Game/CarlaStatics.h"    // GetEpisode
#include "Carla/Sensor/DReyeVRSensor.h" // ADReyeVRSensor::bIsReplaying

#include <string>

std::unordered_map<std::string, class ADReyeVRCustomActor *> ADReyeVRCustomActor::ActiveCustomActors = {};

ADReyeVRCustomActor::ADReyeVRCustomActor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // done in child class
    Internals.Location = this->GetActorLocation();
    Internals.Rotation = this->GetActorRotation();
    Internals.Scale3D = this->GetActorScale3D();
}

void ADReyeVRCustomActor::Initialize(const DReyeVR::CustomActorData &In)
{
    SetInternals(In);
    ADReyeVRCustomActor::ActiveCustomActors[TCHAR_TO_UTF8(*In.Name)] = this;
}

void ADReyeVRCustomActor::BeginPlay()
{
    Super::BeginPlay();
}

void ADReyeVRCustomActor::BeginDestroy()
{
    Super::BeginDestroy();
}

void ADReyeVRCustomActor::Tick(float DeltaSeconds)
{
    if (ADReyeVRSensor::bIsReplaying)
    {
        // update world state with internals
        this->SetActorLocation(Internals.Location);
        this->SetActorRotation(Internals.Rotation);
        this->SetActorScale3D(Internals.Scale3D);
    }
    else
    {
        // update internals with world state
        Internals.Location = this->GetActorLocation();
        Internals.Rotation = this->GetActorRotation();
        Internals.Scale3D = this->GetActorScale3D();
    }
    /// TODO: use other string?
}

void ADReyeVRCustomActor::SetInternals(const DReyeVR::CustomActorData &InData)
{
    Internals = InData;
}

const DReyeVR::CustomActorData &ADReyeVRCustomActor::GetInternals() const
{
    return Internals;
}