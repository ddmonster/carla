#include "DReyeVRFactory.h"
#include "Carla.h"                                     // to avoid linker errors
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h" // UActorBlueprintFunctionLibrary
#include "Carla/Actor/VehicleParameters.h"             // FVehicleParameters
#include "Carla/Game/CarlaEpisode.h"                   // UCarlaEpisode
#include "EgoSensor.h"                                 // AEgoSensor
#include "EgoVehicle.h"                                // AEgoVehicle

// instead of vehicle.dreyevr.model3 or sensor.dreyevr.ego_sensor, we use "harplab" for category
// => harplab.dreyevr_vehicle.model3 & harplab.dreyevr_sensor.ego_sensor
// in PythonAPI use world.get_actors().filter("harplab.dreyevr_vehicle.*") or
// world.get_blueprint_library().filter("harplab.dreyevr_sensor.*") and you won't accidentally get these actors when
// performing filter("vehicle.*") or filter("sensor.*")
#define CATEGORY TEXT("HARPLab")

ADReyeVRFactory::ADReyeVRFactory(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    std::vector<FString> VehicleTypes = {"TeslaM3", "Mustang66", "Jeep", "Vespa"};
    for (const FString &Name : VehicleTypes)
    {
        ConfigFile VehicleParams(FPaths::Combine(CarlaUE4Path, TEXT("Config/EgoVehicles"), Name + ".ini"));
        FString BP_Path;
        if (VehicleParams.bIsValid() && VehicleParams.Get<FString>("Blueprint", "Path", BP_Path))
        {
            ConstructorHelpers::FObjectFinder<UClass> BlueprintObject(*BP_Path);
            BP_Classes.Add(Name, BlueprintObject.Object);
        }
        else
        {
            LOG_WARN("Unable to load custom EgoVehicle \"%s\"", *Name);
            BP_Classes.Add(Name, AEgoVehicle::StaticClass());
        }
    }
}

TArray<FActorDefinition> ADReyeVRFactory::GetDefinitions()
{
    TArray<FActorDefinition> Definitions;

    for (auto &BP_Class_pair : BP_Classes)
    {
        FActorDefinition Def;
        FVehicleParameters Parameters;
        Parameters.Model = BP_Class_pair.Key; // vehicle type
        /// TODO: BP_Path??
        Parameters.ObjectType = BP_Class_pair.Key;
        Parameters.Class = BP_Class_pair.Value;
        /// TODO: manage number of wheels? (though carla's 2-wheeled are just secret 4-wheeled)
        Parameters.NumberOfWheels = 4;

        ADReyeVRFactory::MakeVehicleDefinition(Parameters, Def);
        Definitions.Add(Def);
    }

    FActorDefinition EgoSensorDef;
    {
        const FString Id = "ego_sensor";
        ADReyeVRFactory::MakeSensorDefinition(Id, EgoSensorDef);
        Definitions.Add(EgoSensorDef);
    }

    return Definitions;
}

// copied and modified from UActorBlueprintFunctionLibrary
FActorDefinition MakeGenericDefinition(const FString &Category, const FString &Type, const FString &Id)
{
    FActorDefinition Definition;

    TArray<FString> Tags = {Category.ToLower(), Type.ToLower(), Id.ToLower()};
    Definition.Id = FString::Join(Tags, TEXT("."));
    Definition.Tags = FString::Join(Tags, TEXT(","));
    return Definition;
}

void ADReyeVRFactory::MakeVehicleDefinition(const FVehicleParameters &Parameters, FActorDefinition &Definition)
{
    // assign the ID/Tags with category (ex. "vehicle.tesla.model3" => "harplab.dreyevr.model3")
    Definition = MakeGenericDefinition(CATEGORY, TEXT("DReyeVR_Vehicle"), Parameters.Model);
    Definition.Class = Parameters.Class;

    FActorVariation ActorRole;
    {
        ActorRole.Id = TEXT("role_name");
        ActorRole.Type = EActorAttributeType::String;
        ActorRole.RecommendedValues = {TEXT("ego_vehicle")}; // assume this is the CARLA "hero"
        ActorRole.bRestrictToRecommended = false;
    }
    Definition.Variations.Emplace(ActorRole);

    FActorVariation StickyControl;
    {
        StickyControl.Id = TEXT("sticky_control");
        StickyControl.Type = EActorAttributeType::Bool;
        StickyControl.bRestrictToRecommended = false;
        StickyControl.RecommendedValues.Emplace(TEXT("false"));
    }
    Definition.Variations.Emplace(StickyControl);

    FActorAttribute ObjectType;
    {
        ObjectType.Id = TEXT("object_type");
        ObjectType.Type = EActorAttributeType::String;
        ObjectType.Value = Parameters.ObjectType;
    }
    Definition.Attributes.Emplace(ObjectType);

    FActorAttribute NumberOfWheels;
    {
        NumberOfWheels.Id = TEXT("number_of_wheels");
        NumberOfWheels.Type = EActorAttributeType::Int;
        NumberOfWheels.Value = FString::FromInt(Parameters.NumberOfWheels);
    }
    Definition.Attributes.Emplace(NumberOfWheels);

    FActorAttribute Generation;
    {
        Generation.Id = TEXT("generation");
        Generation.Type = EActorAttributeType::Int;
        Generation.Value = FString::FromInt(Parameters.Generation);
    }
    Definition.Attributes.Emplace(Generation);
}

void ADReyeVRFactory::MakeSensorDefinition(const FString &Id, FActorDefinition &Definition)
{
    Definition = MakeGenericDefinition(CATEGORY, TEXT("DReyeVR_Sensor"), Id);
    Definition.Class = AEgoSensor::StaticClass();
}

FActorSpawnResult ADReyeVRFactory::SpawnActor(const FTransform &SpawnAtTransform,
                                              const FActorDescription &ActorDescription)
{
    auto *World = GetWorld();
    if (World == nullptr)
    {
        LOG_ERROR("cannot spawn \"%s\" into an empty world.", *ActorDescription.Id);
        return {};
    }

    AActor *SpawnedActor = nullptr;

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto SpawnSingleton = [World, SpawnParameters](UClass *ActorClass, const FString &Id, const FTransform &SpawnAt,
                                                   const std::function<AActor *()> &SpawnFn) -> AActor * {
        // function to spawn a singleton: only one actor can exist in the world at once
        ensure(World != nullptr);
        AActor *SpawnedSingleton = nullptr;
        TArray<AActor *> Found;
        UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Found);
        if (Found.Num() == 0)
        {
            LOG("Spawning DReyeVR actor (\"%s\") at: %s", *Id, *SpawnAt.ToString());
            SpawnedSingleton = SpawnFn();
        }
        else
        {
            LOG_WARN("Requested to spawn another DReyeVR actor (\"%s\") but one already exists in the world!", *Id);
            ensure(Found.Num() == 1); // should only have one other that was previously spawned
            SpawnedSingleton = Found[0];
        }
        return SpawnedSingleton;
    };

    if (UClass::FindCommonBase(ActorDescription.Class, AEgoVehicle::StaticClass()) ==
        AEgoVehicle::StaticClass()) // is EgoVehicle or derived class
    {
        // see if this requested actor description is one of the available EgoVehicles
        /// NOTE: multi-ego-vehicle is not officially supported by DReyeVR, but it could be an interesting extension
        for (const auto &AvailableEgoVehicles : BP_Classes)
        {
            const FString &Name = AvailableEgoVehicles.Key;
            if (ActorDescription.Id.ToLower().Contains(Name.ToLower())) // contains name
            {
                // check if an EgoVehicle already exists, if so, don't spawn another.
                SpawnedActor = SpawnSingleton(ActorDescription.Class, ActorDescription.Id, SpawnAtTransform, [&]() {
                    auto *Class = AvailableEgoVehicles.Value;
                    return World->SpawnActor<AEgoVehicle>(Class, SpawnAtTransform, SpawnParameters);
                });
            }
        }
    }
    else if (ActorDescription.Class == AEgoSensor::StaticClass())
    {
        // there should only ever be one DReyeVR sensor in the world!
        SpawnedActor = SpawnSingleton(ActorDescription.Class, ActorDescription.Id, SpawnAtTransform, [&]() {
            return World->SpawnActor<AEgoSensor>(ActorDescription.Class, SpawnAtTransform, SpawnParameters);
        });
    }
    else
    {
        LOG_ERROR("Unknown actor class in DReyeVR factory!");
    }

    if (SpawnedActor == nullptr)
    {
        LOG_WARN("Unable to spawn DReyeVR actor (\"%s\")", *ActorDescription.Id)
    }
    return FActorSpawnResult(SpawnedActor);
}
