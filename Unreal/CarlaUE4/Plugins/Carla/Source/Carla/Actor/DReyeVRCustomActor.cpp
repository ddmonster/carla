#include "DReyeVRCustomActor.h"
#include "Carla/Game/CarlaStatics.h"           // GetEpisode
#include "Carla/Sensor/DReyeVRSensor.h"        // ADReyeVRSensor::bIsReplaying
#include "Materials/MaterialInstance.h"        // UMaterialInstance
#include "Materials/MaterialInstanceDynamic.h" // UMaterialInstanceDynamic
#include "UObject/ConstructorHelpers.h"        // ConstructorHelpers
#include "UObject/UObjectGlobals.h"            // LoadObject

#include <string>

std::unordered_map<std::string, class ADReyeVRCustomActor *> ADReyeVRCustomActor::ActiveCustomActors = {};
int ADReyeVRCustomActor::AllMeshCount = 0;
const FString ADReyeVRCustomActor::OpaqueMaterial =
    "Material'/Game/DReyeVR/Custom/OpaqueParamMaterial.OpaqueParamMaterial'";
const FString ADReyeVRCustomActor::TransparentMaterial =
    "Material'/Game/DReyeVR/Custom/TransparentParamMaterial.TransparentParamMaterial'";

ADReyeVRCustomActor::ADReyeVRCustomActor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // turning off physics interaction
    this->SetActorEnableCollision(false);

    // done in child class
    Internals.Location = this->GetActorLocation();
    Internals.Rotation = this->GetActorRotation();
    Internals.Scale3D = this->GetActorScale3D();
}

void ADReyeVRCustomActor::AssignSM(const FString &Path)
{
    FName MeshName(*("Mesh" + FString::FromInt(ADReyeVRCustomActor::AllMeshCount)));
    ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(MeshName);
    ActorMesh->SetupAttachment(this->GetRootComponent());
    this->SetRootComponent(ActorMesh);

    ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(*Path);
    if (MeshAsset.Succeeded())
        ActorMesh->SetStaticMesh(MeshAsset.Object);
    else
        UE_LOG(LogTemp, Error, TEXT("Unable to access mesh asset: %s"), *Path)
    // static count for how many meshes have been processed thus far
    ADReyeVRCustomActor::AllMeshCount++;
}

void ADReyeVRCustomActor::AssignMat(const FString &Path)
{
    UMaterial *Material = LoadObject<UMaterial>(nullptr, *Path);
    ensure(Material != nullptr);

    // create sole dynamic material
    DynamicMat = UMaterialInstanceDynamic::Create(Material, this);
    MaterialParams.Apply(DynamicMat);
    MaterialParams.MaterialPath = Path;

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
    if (DynamicMat)
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