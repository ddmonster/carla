#include "DReyeVRFactory.h"
#include "Carla.h"                                     // to avoid linker errors
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h" // UActorBlueprintFunctionLibrary
#include "Carla/Actor/VehicleParameters.h"             // FVehicleParameters
#include "Carla/Game/CarlaEpisode.h"                   // UCarlaEpisode
#include "EgoSensor.h"                                 // AEgoSensor
#include "EgoVehicle.h"                                // AEgoVehicle

#define EgoVehicleBP_Str "/Game/DReyeVR/EgoVehicle/BP_model3.BP_model3_C"

// instead of vehicle.dreyevr.model3 or sensor.dreyevr.ego_sensor, we use "harplab" for category
// => dreyevr.vehicle.model3 & dreyevr.sensor.ego_sensor
// in PythonAPI use world.get_actors().filter("dreyevr.*") or world.get_blueprint_library().filter("dreyevr.*")
// and you won't accidentally get these actors when performing filter("vehicle.*") or filter("sensor.*")
#define CATEGORY TEXT("DReyeVR")

ADReyeVRFactory::ADReyeVRFactory(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // https://forums.unrealengine.com/t/cdo-constructor-failed-to-find-thirdperson-c-template-mannequin-animbp/99003
    // get ego vehicle bp (can use UTF8_TO_TCHAR if making EgoVehicleBP_Str a variable)
    static ConstructorHelpers::FObjectFinder<UClass> EgoVehicleBP(TEXT(EgoVehicleBP_Str));
    EgoVehicleBPClass = EgoVehicleBP.Object;
    ensure(EgoVehicleBPClass != nullptr);
}

TArray<FActorDefinition> ADReyeVRFactory::GetDefinitions()
{
    FActorDefinition EgoVehicleDef;
    {
        FVehicleParameters Parameters;
        Parameters.Make = "Vehicle";
        Parameters.Model = "Model3";
        Parameters.ObjectType = EgoVehicleBP_Str;
        Parameters.Class = AEgoVehicle::StaticClass();
        Parameters.NumberOfWheels = 4;

        // need to create an FActorDefinition from our FActorDescription for some reason -_-
        bool Success = false;
        ADReyeVRFactory::MakeVehicleDefinition(Parameters, Success, EgoVehicleDef);
        if (!Success)
        {
            LOG_ERROR("Unable to create DReyeVR vehicle definition!");
        }
        EgoVehicleDef.Class = Parameters.Class;
    }

    FActorDefinition EgoSensorDef;
    {
        const FString Type = "Sensor";
        const FString Id = "Ego_Sensor";
        ADReyeVRFactory::MakeSensorDefinition(Type, Id, EgoSensorDef);
        EgoSensorDef.Class = AEgoSensor::StaticClass();
    }

    return {EgoVehicleDef, EgoSensorDef};
}

void ADReyeVRFactory::MakeVehicleDefinition(const FVehicleParameters &Parameters, bool &Success,
                                            FActorDefinition &Definition)
{
    // assign the ID/Tags with category (ex. "vehicle.tesla.model3" => "harplab.dreyevr.model3")
    Definition = UActorBlueprintFunctionLibrary::MakeGenericDefinition(CATEGORY, Parameters.Make, Parameters.Model);
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

    Success = UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition);
}

void ADReyeVRFactory::MakeSensorDefinition(const FString &Type, const FString &Id, FActorDefinition &Definition)
{
    Definition = UActorBlueprintFunctionLibrary::MakeGenericDefinition(CATEGORY, Type, Id);
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
    if (ActorDescription.Class == AEgoVehicle::StaticClass())
    {
        LOG("Spawning EgoVehicle (\"%s\") at: %s", *ActorDescription.Id, *SpawnAtTransform.ToString());
        SpawnedActor = World->SpawnActor<AEgoVehicle>(EgoVehicleBPClass, SpawnAtTransform, SpawnParameters);
    }
    else if (ActorDescription.Class == AEgoSensor::StaticClass())
    {
        LOG("Spawning EgoSensor (\"%s\") at: %s", *ActorDescription.Id, *SpawnAtTransform.ToString());
        SpawnedActor = World->SpawnActor<AEgoSensor>(ActorDescription.Class, SpawnAtTransform, SpawnParameters);
    }
    else
    {
        LOG_ERROR("Unknown actor class in DReyeVR factory!");
    }
    return FActorSpawnResult(SpawnedActor);
}
