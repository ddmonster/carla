#include "DReyeVRCustomActor.h"
#include "Carla/Game/CarlaStatics.h"           // GetEpisode
#include "Carla/Sensor/DReyeVRSensor.h"        // ADReyeVRSensor::bIsReplaying
#include "Materials/MaterialInstance.h"        // UMaterialInstance
#include "Materials/MaterialInstanceDynamic.h" // UMaterialInstanceDynamic
#include "UObject/UObjectGlobals.h"            // LoadObject, NewObject

#include <string>

std::unordered_map<std::string, class ADReyeVRCustomActor *> ADReyeVRCustomActor::ActiveCustomActors = {};
int ADReyeVRCustomActor::AllMeshCount = 0;
const FString ADReyeVRCustomActor::OpaqueMaterial =
    "Material'/Game/DReyeVR/Custom/OpaqueParamMaterial.OpaqueParamMaterial'";
const FString ADReyeVRCustomActor::TranslucentMaterial =
    "Material'/Game/DReyeVR/Custom/TranslucentParamMaterial.TranslucentParamMaterial'";

ADReyeVRCustomActor *ADReyeVRCustomActor::CreateNew(DReyeVR::CustomActorData::Types RequestedType, UWorld *World,
                                                    const FString &Name, const int KnownNumMaterials)
{
    check(World != nullptr);
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ADReyeVRCustomActor *Actor =
        World->SpawnActor<ADReyeVRCustomActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

    FString SM_Path;
    switch (RequestedType)
    {
    case DReyeVR::CustomActorData::Types::SPHERE:
        SM_Path = "StaticMesh'/Engine/BasicShapes/Sphere.Sphere'";
        break;
    case DReyeVR::CustomActorData::Types::CUBE:
        SM_Path = "StaticMesh'/Engine/BasicShapes/Cube.Cube'";
        break;
    case DReyeVR::CustomActorData::Types::CONE:
        SM_Path = "StaticMesh'/Engine/BasicShapes/Cone.Cone'";
        break;
    case DReyeVR::CustomActorData::Types::CROSS:
        SM_Path = "StaticMesh'/Game/DReyeVR/Custom/Periph/SM_FixationCross.SM_FixationCross'";
        break;
    case DReyeVR::CustomActorData::Types::ARROW:
        /// TODO: make Arrow static mesh
        SM_Path = "StaticMesh'/Engine/BasicShapes/Cube.Cube'";
        break;
    /// TODO: generalize for other types (templates?? :eyes:)
    default:
        UE_LOG(LogTemp, Warning, TEXT("Unknown actor type: %d"), int(RequestedType));
        break; // ignore unknown actors
    }
    Actor->AssignSM(SM_Path, World);
    Actor->NumMaterials = KnownNumMaterials;
    Actor->Initialize(Name);
    return Actor;
}

ADReyeVRCustomActor::ADReyeVRCustomActor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // turning off physics interaction
    this->SetActorEnableCollision(false);

    // done in child class
    Internals.Location = this->GetActorLocation();
    Internals.Rotation = this->GetActorRotation();
    Internals.Scale3D = this->GetActorScale3D();
}

void ADReyeVRCustomActor::AssignSM(const FString &Path, UWorld *World)
{
    UStaticMesh *SM = LoadObject<UStaticMesh>(nullptr, *Path);
    ensure(SM != nullptr);
    ensure(World != nullptr);
    if (SM)
    {
        // Using static AllMeshCount to create a unique component name every time
        FName MeshName(*("Mesh" + FString::FromInt(ADReyeVRCustomActor::AllMeshCount)));
        ActorMesh = NewObject<UStaticMeshComponent>(this, MeshName);
        ensure(ActorMesh != nullptr);
        ActorMesh->SetupAttachment(this->GetRootComponent());
        this->SetRootComponent(ActorMesh);
        ActorMesh->SetStaticMesh(SM);
        ActorMesh->SetVisibility(false);
        ActorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ActorMesh->RegisterComponentWithWorld(World);
        this->AddInstanceComponent(ActorMesh);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to find mesh: %s"), *Path);
    }
    ADReyeVRCustomActor::AllMeshCount++;
}

void ADReyeVRCustomActor::AssignMat(const FString &Path)
{
    // Path should be one of {ADReyeVRCustomActor::OpaqueMaterial, ADReyeVRCustomActor::TranslucentMaterial}
    UMaterial *Material = LoadObject<UMaterial>(nullptr, *Path);
    ensure(Material != nullptr);

    // create sole dynamic material
    DynamicMat = UMaterialInstanceDynamic::Create(Material, this);
    MaterialParams.Apply(DynamicMat);   // apply the parameters to this dynamic material
    MaterialParams.MaterialPath = Path; // for now does not change over time

    if (DynamicMat != nullptr && ActorMesh != nullptr)
        for (int i = 0; i < NumMaterials; i++)
            ActorMesh->SetMaterial(i, DynamicMat);
    else
        UE_LOG(LogTemp, Error, TEXT("Unable to access material asset: %s"), *Path)
}

void ADReyeVRCustomActor::Initialize(const FString &Name)
{
    Internals.Name = Name;
    ADReyeVRCustomActor::ActiveCustomActors[TCHAR_TO_UTF8(*Name)] = this;
    UE_LOG(LogTemp, Log, TEXT("Initialized custom actor: %s"), *Name);
}

void ADReyeVRCustomActor::BeginPlay()
{
    Super::BeginPlay();
}

void ADReyeVRCustomActor::BeginDestroy()
{
    this->Deactivate(); // remove from global static table
    Super::BeginDestroy();
}

void ADReyeVRCustomActor::Deactivate()
{
    const std::string s = TCHAR_TO_UTF8(*Internals.Name);
    // UE_LOG(LogTemp, Log, TEXT("Disabling custom actor: %s"), *Internals.Name);
    if (ADReyeVRCustomActor::ActiveCustomActors.find(s) != ADReyeVRCustomActor::ActiveCustomActors.end())
    {
        ADReyeVRCustomActor::ActiveCustomActors.erase(s);
    }
    this->SetActorHiddenInGame(true);
    if (ActorMesh)
        ActorMesh->SetVisibility(false);
    this->SetActorTickEnabled(false);
    this->bIsActive = false;
}

void ADReyeVRCustomActor::Activate()
{
    // UE_LOG(LogTemp, Log, TEXT("Enabling custom actor: %s"), *Internals.Name);
    const std::string s = TCHAR_TO_UTF8(*Internals.Name);
    if (ADReyeVRCustomActor::ActiveCustomActors.find(s) == ADReyeVRCustomActor::ActiveCustomActors.end())
        ADReyeVRCustomActor::ActiveCustomActors[s] = this;
    else
        ensure(ADReyeVRCustomActor::ActiveCustomActors[s] == this);
    this->SetActorHiddenInGame(false);
    if (ActorMesh)
        ActorMesh->SetVisibility(true);
    this->SetActorTickEnabled(true);
    this->bIsActive = true;
}

void ADReyeVRCustomActor::Tick(float DeltaSeconds)
{
    if (ADReyeVRSensor::bIsReplaying)
    {
        // update world state with internals
        this->SetActorLocation(Internals.Location);
        this->SetActorRotation(Internals.Rotation);
        this->SetActorScale3D(Internals.Scale3D);
        this->MaterialParams = Internals.MaterialParams;
    }
    else
    {
        // update internals with world state
        Internals.Location = this->GetActorLocation();
        Internals.Rotation = this->GetActorRotation();
        Internals.Scale3D = this->GetActorScale3D();
        Internals.MaterialParams = MaterialParams;
    }
    // update the materials according to the params
    MaterialParams.Apply(DynamicMat);
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