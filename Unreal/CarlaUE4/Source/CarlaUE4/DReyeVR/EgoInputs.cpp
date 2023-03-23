#include "EgoVehicle.h"
#include "Math/NumericLimits.h" // TNumericLimits<float>::Max
#include <string>               // std::string, std::wstring

////////////////:INPUTS:////////////////
/// NOTE: Here we define all the Input functions for the EgoVehicle just to keep them
// from cluttering the EgoVehcile.cpp file

const DReyeVR::UserInputs &AEgoVehicle::GetVehicleInputs() const
{
    return VehicleInputs;
}

const FTransform &AEgoVehicle::GetCameraRootPose() const
{
    return CameraPoseOffset;
}

void AEgoVehicle::CameraFwd()
{
    // move by (1, 0, 0)
    CameraPositionAdjust(FVector::ForwardVector);
}

void AEgoVehicle::CameraBack()
{
    // move by (-1, 0, 0)
    CameraPositionAdjust(FVector::BackwardVector);
}

void AEgoVehicle::CameraLeft()
{
    // move by (0, -1, 0)
    CameraPositionAdjust(FVector::LeftVector);
}

void AEgoVehicle::CameraRight()
{
    // move by (0, 1, 0)
    CameraPositionAdjust(FVector::RightVector);
}

void AEgoVehicle::CameraUp()
{
    // move by (0, 0, 1)
    CameraPositionAdjust(FVector::UpVector);
}

void AEgoVehicle::CameraDown()
{
    // move by (0, 0, -1)
    CameraPositionAdjust(FVector::DownVector);
}

void AEgoVehicle::CameraPositionAdjust(const FVector &Disp)
{
    if (Disp.Equals(FVector::ZeroVector, 0.0001f))
        return;
    // preserves adjustment even after changing view
    CameraPoseOffset.SetLocation(CameraPoseOffset.GetLocation() + Disp);
    VRCameraRoot->SetRelativeLocation(CameraPose.GetLocation() + CameraPoseOffset.GetLocation());
    /// TODO: account for rotation? scale?
}

void AEgoVehicle::CameraPositionAdjust(bool bForward, bool bRight, bool bBackwards, bool bLeft, bool bUp, bool bDown)
{
    // add the corresponding directions according to the adjustment booleans
    const FVector Disp = FVector::ForwardVector * bForward + FVector::RightVector * bRight +
                         FVector::BackwardVector * bBackwards + FVector::LeftVector * bLeft + FVector::UpVector * bUp +
                         FVector::DownVector * bDown;
    CameraPositionAdjust(Disp);
}

void AEgoVehicle::PressNextCameraView()
{
    if (!bCanPressNextCameraView)
        return;
    bCanPressNextCameraView = false;
    NextCameraView();
};
void AEgoVehicle::ReleaseNextCameraView()
{
    bCanPressNextCameraView = true;
};

void AEgoVehicle::PressPrevCameraView()
{
    if (!bCanPressPrevCameraView)
        return;
    bCanPressPrevCameraView = false;
    PrevCameraView();
};
void AEgoVehicle::ReleasePrevCameraView()
{
    bCanPressPrevCameraView = true;
};

void AEgoVehicle::NextCameraView()
{
    CurrentCameraTransformIdx = (CurrentCameraTransformIdx + 1) % (CameraTransforms.size());
    LOG("Switching to next camera view: \"%s\"", *CameraTransforms[CurrentCameraTransformIdx].first);
    SetCameraRootPose(CurrentCameraTransformIdx);
}

void AEgoVehicle::PrevCameraView()
{
    if (CurrentCameraTransformIdx == 0)
        CurrentCameraTransformIdx = CameraTransforms.size();
    CurrentCameraTransformIdx--;
    LOG("Switching to prev camera view: \"%s\"", *CameraTransforms[CurrentCameraTransformIdx].first);
    SetCameraRootPose(CurrentCameraTransformIdx);
}

void AEgoVehicle::AddSteering(const float SteeringInput)
{
    float ScaledSteeringInput = this->ScaleSteeringInput * SteeringInput;
    // assign to input struct
    VehicleInputs.Steering += ScaledSteeringInput;
}

void AEgoVehicle::AddThrottle(const float ThrottleInput)
{
    float ScaledThrottleInput = this->ScaleThrottleInput * ThrottleInput;

    // apply new light state
    FVehicleLightState Lights = GetVehicleLightState();
    Lights.Reverse = false;
    Lights.Brake = false;
    SetVehicleLightState(Lights);

    // assign to input struct
    VehicleInputs.Throttle += ScaledThrottleInput;
}

void AEgoVehicle::AddBrake(const float BrakeInput)
{
    float ScaledBrakeInput = this->ScaleBrakeInput * BrakeInput;

    // apply new light state
    FVehicleLightState Lights = GetVehicleLightState();
    Lights.Reverse = false;
    Lights.Brake = true;
    SetVehicleLightState(Lights);

    // assign to input struct
    VehicleInputs.Brake += ScaledBrakeInput;
}

void AEgoVehicle::PressReverse()
{
    if (!bCanPressReverse)
        return;
    bCanPressReverse = false; // don't press again until release
    bReverse = !bReverse;

    // negate to toggle bw + (forwards) and - (backwards)
    // const int CurrentGear = this->GetVehicleMovementComponent()->GetTargetGear();
    // int NewGear = -1; // for when parked
    // if (CurrentGear != 0)
    // {
    //     NewGear = bReverse ? -1 * std::abs(CurrentGear) : std::abs(CurrentGear); // negative => backwards
    // }
    // this->GetVehicleMovementComponent()->SetTargetGear(NewGear, true); // UE4 control

    // apply new light state
    FVehicleLightState Lights = this->GetVehicleLightState();
    Lights.Reverse = this->bReverse;
    this->SetVehicleLightState(Lights);

    // LOG("Toggle Reverse");
    // assign to input struct
    VehicleInputs.ToggledReverse = true;

    PlayGearShiftSound();

    // call to parent
    SetReverse(bReverse);
}

void AEgoVehicle::ReleaseReverse()
{
    VehicleInputs.ToggledReverse = false;
    bCanPressReverse = true;
}

void AEgoVehicle::PressTurnSignalR()
{
    if (!bCanPressTurnSignalR)
        return;
    bCanPressTurnSignalR = false; // don't press again until release
    // store in local input container
    VehicleInputs.TurnSignalRight = true;

    if (bEnableTurnSignalAction)
    {
        // apply new light state
        FVehicleLightState Lights = this->GetVehicleLightState();
        Lights.RightBlinker = true;
        Lights.LeftBlinker = false;
        this->SetVehicleLightState(Lights);
        // play sound
        this->PlayTurnSignalSound();
    }
    RightSignalTimeToDie = TNumericLimits<float>::Max(); // wait until button released (+inf until then)
    LeftSignalTimeToDie = 0.f;                           // immediately stop left signal
}

void AEgoVehicle::ReleaseTurnSignalR()
{
    if (bCanPressTurnSignalR)
        return;
    VehicleInputs.TurnSignalRight = false;
    RightSignalTimeToDie = GetWorld()->GetTimeSeconds() + this->TurnSignalDuration; // reset counter
    bCanPressTurnSignalR = true;
}

void AEgoVehicle::PressTurnSignalL()
{
    if (!bCanPressTurnSignalL)
        return;
    bCanPressTurnSignalL = false; // don't press again until release
    // store in local input container
    VehicleInputs.TurnSignalLeft = true;

    if (bEnableTurnSignalAction)
    {
        // apply new light state
        FVehicleLightState Lights = this->GetVehicleLightState();
        Lights.RightBlinker = false;
        Lights.LeftBlinker = true;
        this->SetVehicleLightState(Lights);
        // play sound
        this->PlayTurnSignalSound();
    }
    RightSignalTimeToDie = 0.f;                         // immediately stop right signal
    LeftSignalTimeToDie = TNumericLimits<float>::Max(); // wait until button released (+inf until then)
}

void AEgoVehicle::ReleaseTurnSignalL()
{
    if (bCanPressTurnSignalL)
        return;
    VehicleInputs.TurnSignalLeft = false;
    LeftSignalTimeToDie = GetWorld()->GetTimeSeconds() + this->TurnSignalDuration; // reset counter
    bCanPressTurnSignalL = true;
}

void AEgoVehicle::TickVehicleInputs()
{
    SetSteeringInput(VehicleInputs.Steering);
    SetBrakeInput(VehicleInputs.Brake);
    SetThrottleInput(VehicleInputs.Throttle);

    // send these inputs to the Carla (parent) vehicle
    FlushVehicleControl();
    VehicleInputs = DReyeVR::UserInputs(); // clear inputs for this frame
}

/// ========================================== ///
/// ----------------:STATIC:------------------ ///
/// ========================================== ///

void AEgoVehicle::ConstructRigidBody()
{

    // https://forums.unrealengine.com/t/cannot-create-vehicle-updatedcomponent-has-not-initialized-its-rigid-body-actor/461662
    /// NOTE: this must be run in the constructors!

    // load skeletal mesh (static mesh)
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(
        TEXT("SkeletalMesh'/Game/DReyeVR/EgoVehicle/model3/Mesh/SkeletalMesh_model3.SkeletalMesh_model3'"));
    // original: "SkeletalMesh'/Game/Carla/Static/Car/4Wheeled/Tesla/SM_TeslaM3_v2.SM_TeslaM3_v2'"
    USkeletalMesh *SkeletalMesh = CarMesh.Object;
    if (SkeletalMesh == nullptr)
    {
        LOG_ERROR("Unable to load skeletal mesh!");
        return;
    }

    // load skeleton (for animations)
    static ConstructorHelpers::FObjectFinder<USkeleton> CarSkeleton(
        TEXT("Skeleton'/Game/DReyeVR/EgoVehicle/model3/Mesh/Skeleton_model3.Skeleton_model3'"));
    // original:
    // "Skeleton'/Game/Carla/Static/Car/4Wheeled/Tesla/SM_TeslaM3_lights_body_Skeleton.SM_TeslaM3_lights_body_Skeleton'"
    USkeleton *Skeleton = CarSkeleton.Object;
    if (Skeleton == nullptr)
    {
        LOG_ERROR("Unable to load skeleton!");
        return;
    }

    // load animations bp
    static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(
        TEXT("/Game/DReyeVR/EgoVehicle/model3/Mesh/Animation_model3.Animation_model3_C"));
    // original: "/Game/Carla/Static/Car/4Wheeled/Tesla/Tesla_Animation.Tesla_Animation_C"
    auto AnimInstance = AnimBPClass.Class;
    if (!AnimBPClass.Succeeded())
    {
        LOG_ERROR("Unable to load Animation!");
        return;
    }

    // load physics asset
    static ConstructorHelpers::FObjectFinder<UPhysicsAsset> CarPhysics(
        TEXT("PhysicsAsset'/Game/DReyeVR/EgoVehicle/model3/Mesh/Physics_model3.Physics_model3'"));
    // original: "PhysicsAsset'/Game/Carla/Static/Car/4Wheeled/Tesla/SM_TeslaM3_PhysicsAsset.SM_TeslaM3_PhysicsAsset'"
    UPhysicsAsset *PhysicsAsset = CarPhysics.Object;
    if (PhysicsAsset == nullptr)
    {
        LOG_ERROR("Unable to load PhysicsAsset!");
        return;
    }

    // contrary to https://docs.unrealengine.com/4.27/en-US/API/Runtime/Engine/Engine/USkeletalMesh/SetSkeleton/
    SkeletalMesh->Skeleton = Skeleton;
    SkeletalMesh->PhysicsAsset = PhysicsAsset;
    SkeletalMesh->Build();

    USkeletalMeshComponent *Mesh = GetMesh();
    check(Mesh != nullptr);
    Mesh->SetSkeletalMesh(SkeletalMesh, true);
    Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    Mesh->SetAnimInstanceClass(AnimInstance);
    Mesh->SetPhysicsAsset(PhysicsAsset);
    Mesh->RecreatePhysicsState();
    this->GetVehicleMovementComponent()->RecreatePhysicsState();

    // sanity checks
    ensure(Mesh->GetPhysicsAsset() != nullptr);

    // set up wheels
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());
    check(Vehicle4W != nullptr);
    check(Vehicle4W->WheelSetups.Num() == 4);
    // Wheels/Tyres
    /// TODO: ensure the wheels bonename matches the Carla SkeletalMesh
    // Setup the wheels
    Vehicle4W->WheelSetups[0].WheelClass = UVehicleWheel::StaticClass();
    Vehicle4W->WheelSetups[0].BoneName = FName("Wheel_Front_Left");
    // Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -8.f, 0.f);

    Vehicle4W->WheelSetups[1].WheelClass = UVehicleWheel::StaticClass();
    Vehicle4W->WheelSetups[1].BoneName = FName("Wheel_Front_Right");
    // Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 8.f, 0.f);

    Vehicle4W->WheelSetups[2].WheelClass = UVehicleWheel::StaticClass();
    Vehicle4W->WheelSetups[2].BoneName = FName("Wheel_Rear_Left");
    Vehicle4W->WheelSetups[2].bDisableSteering = true;
    // Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -8.f, 0.f);

    Vehicle4W->WheelSetups[3].WheelClass = UVehicleWheel::StaticClass();
    Vehicle4W->WheelSetups[3].BoneName = FName("Wheel_Rear_Right");
    Vehicle4W->WheelSetups[3].bDisableSteering = true;
    // Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 8.f, 0.f);

    // Adjust the tire loading
    Vehicle4W->MinNormalizedTireLoad = 0.0f;
    Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
    Vehicle4W->MaxNormalizedTireLoad = 2.0f;
    Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

    // Engine
    // Torque setup
    Vehicle4W->MaxEngineRPM = 5700.0f;
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);

    // Adjust the steering
    Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

    // Transmission
    // We want 4wd
    Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_RearDrive;

    // Drive the front wheels a little more than the rear
    Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65;

    // Automatic gearbox
    Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;

    LOG("Successfully created EgoVehicle rigid body");
}

FVehiclePhysicsControl AEgoVehicle::GetVehiclePhysicsControl() const
{
    // taken mostly from Carla
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());
    check(Vehicle4W != nullptr);

    FVehiclePhysicsControl PhysicsControl;

    // Engine Setup
    PhysicsControl.TorqueCurve = Vehicle4W->EngineSetup.TorqueCurve.EditorCurveData;
    PhysicsControl.MaxRPM = Vehicle4W->EngineSetup.MaxRPM;
    PhysicsControl.MOI = Vehicle4W->EngineSetup.MOI;
    PhysicsControl.DampingRateFullThrottle = Vehicle4W->EngineSetup.DampingRateFullThrottle;
    PhysicsControl.DampingRateZeroThrottleClutchEngaged = Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchEngaged;
    PhysicsControl.DampingRateZeroThrottleClutchDisengaged =
        Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchDisengaged;

    // Transmission Setup
    PhysicsControl.bUseGearAutoBox = Vehicle4W->TransmissionSetup.bUseGearAutoBox;
    PhysicsControl.GearSwitchTime = Vehicle4W->TransmissionSetup.GearSwitchTime;
    PhysicsControl.ClutchStrength = Vehicle4W->TransmissionSetup.ClutchStrength;
    PhysicsControl.FinalRatio = Vehicle4W->TransmissionSetup.FinalRatio;

    TArray<FGearPhysicsControl> ForwardGears;

    for (const auto &Gear : Vehicle4W->TransmissionSetup.ForwardGears)
    {
        FGearPhysicsControl GearPhysicsControl;

        GearPhysicsControl.Ratio = Gear.Ratio;
        GearPhysicsControl.UpRatio = Gear.UpRatio;
        GearPhysicsControl.DownRatio = Gear.DownRatio;

        ForwardGears.Add(GearPhysicsControl);
    }

    PhysicsControl.ForwardGears = ForwardGears;

    // Vehicle Setup
    PhysicsControl.Mass = Vehicle4W->Mass;
    PhysicsControl.DragCoefficient = Vehicle4W->DragCoefficient;

    // Center of mass offset (Center of mass is always zero vector in local
    // position)
    UPrimitiveComponent *UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
    check(UpdatedPrimitive != nullptr);

    PhysicsControl.CenterOfMass = UpdatedPrimitive->BodyInstance.COMNudge;

    // Transmission Setup
    PhysicsControl.SteeringCurve = Vehicle4W->SteeringCurve.EditorCurveData;

    return PhysicsControl;
}

void AEgoVehicle::SetWheelsFrictionScale(TArray<float> &WheelsFrictionScale)
{
    /// TODO: implement correctly
    // currently have a crash when parent calls:
    // Vehicle4W->Wheels[i]->TireConfig->SetFrictionScale(WheelsFrictionScale[i]);
}