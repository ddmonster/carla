#include "EgoVehicle.h"
#include "Math/NumericLimits.h" // TNumericLimits<float>::Max
#include "TireConfig.h"         // UTireConfig
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
    if (CameraPoseKeys.Num() == 0)
        return;
    CurrentCameraTransformIdx = (CurrentCameraTransformIdx + 1) % (CameraPoseKeys.Num());
    const FString &Key = CameraPoseKeys[CurrentCameraTransformIdx];
    LOG("Switching manually to next camera view: \"%s\"", *Key);
    SetCameraRootPose(CurrentCameraTransformIdx);
}

void AEgoVehicle::PrevCameraView()
{
    if (CameraPoseKeys.Num() == 0)
        return;
    if (CurrentCameraTransformIdx == 0)
        CurrentCameraTransformIdx = CameraPoseKeys.Num();
    CurrentCameraTransformIdx--;
    const FString &Key = CameraPoseKeys[CurrentCameraTransformIdx];
    LOG("Switching manually to prev camera view: \"%s\"", *Key);
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

    // if you'd like to manage everything in code, you can specify all the paths for the rigid body
    // components (skeletal mesh, skeleton, animation, and physics asset) then you should not include
    // a Blueprint path to work on. If you do, we assume you are already working in the Editor so there
    // is no need for us to re-assign these parameters here.
    const FString BlueprintPath = VehicleParams.Get<FString>("Blueprint", "Path");
    if (!BlueprintPath.IsEmpty())
    {
        LOG("Successfully loaded EgoVehicle from \"%s\"", *BlueprintPath);
        return; // already working in the editor with a blueprint
    }

    // https://forums.unrealengine.com/t/cannot-create-vehicle-updatedcomponent-has-not-initialized-its-rigid-body-actor/461662
    /// NOTE: this must be run in the constructors!

    USkeletalMesh *SkeletalMesh = nullptr;
    {
        // load skeletal mesh (static mesh)
        const FString SkeletalMeshPath = UE4RefToClassPath(VehicleParams.Get<FString>("RigidBody", "SkeletalMesh"));
        ConstructorHelpers::FObjectFinder<UClass> CarMesh(*SkeletalMeshPath);
        SkeletalMesh = Cast<USkeletalMesh>(CarMesh.Object);
        if (SkeletalMesh == nullptr)
        {
            LOG_ERROR("Unable to load skeletal mesh! (\"%s\")", *SkeletalMeshPath);
        }
    }

    USkeleton *Skeleton = nullptr;
    {
        // load skeleton (for animations)
        const FString SkeletonPath = UE4RefToClassPath(VehicleParams.Get<FString>("RigidBody", "Skeleton"));
        ConstructorHelpers::FObjectFinder<UClass> CarSkeleton(*SkeletonPath);
        Skeleton = Cast<USkeleton>(CarSkeleton.Object);
        if (Skeleton == nullptr)
        {
            LOG_ERROR("Unable to load skeleton! (\"%s\")", *SkeletonPath);
        }
    }

    UClass *AnimInstanceClass = nullptr;
    {
        // load animations bp
        const FString AnimationPath = UE4RefToClassPath(VehicleParams.Get<FString>("RigidBody", "AnimationBP"));
        // format should be /Game/.../type.type_C (remove prefix, quotes, and add "_C" suffix)
        FString AnimationPathObj = UE4RefToClassPath(AnimationPath);
        ConstructorHelpers::FClassFinder<UObject> AnimClass(*AnimationPathObj);
        if (AnimClass.Succeeded())
        {
            AnimInstanceClass = AnimClass.Class;
        }
        else
        {
            LOG_ERROR("Unable to load Animation!");
        }
    }

    UPhysicsAsset *PhysicsAsset = nullptr;
    {
        // load physics asset
        const FString PhysicsAssetPath = UE4RefToClassPath(VehicleParams.Get<FString>("RigidBody", "PhysicsAsset"));
        ConstructorHelpers::FObjectFinder<UClass> CarPhysics(*PhysicsAssetPath);
        PhysicsAsset = Cast<UPhysicsAsset>(CarPhysics.Object);
        if (PhysicsAsset == nullptr)
        {
            LOG_ERROR("Unable to load PhysicsAsset! (\"%s\")", *PhysicsAssetPath);
        }
    }

    if (SkeletalMesh)
    {
        // contrary to https://docs.unrealengine.com/4.27/en-US/API/Runtime/Engine/Engine/USkeletalMesh/SetSkeleton/
        SkeletalMesh->Skeleton = Skeleton ? Skeleton : SkeletalMesh->Skeleton;
        SkeletalMesh->PhysicsAsset = PhysicsAsset ? PhysicsAsset : SkeletalMesh->PhysicsAsset;
    }

    USkeletalMeshComponent *Mesh = GetMesh();
    ensure(Mesh != nullptr);
    if (Mesh)
    {
        if (SkeletalMesh)
            Mesh->SetSkeletalMesh(SkeletalMesh, true);
        if (AnimInstanceClass)
        {
            Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            Mesh->SetAnimInstanceClass(AnimInstanceClass);
        }
        if (PhysicsAsset)
        {
            Mesh->SetPhysicsAsset(PhysicsAsset);
            Mesh->RecreatePhysicsState();
            this->GetVehicleMovementComponent()->RecreatePhysicsState();

            // sanity checks
            ensure(Mesh->GetPhysicsAsset() != nullptr);
        }
    }

    SetupWheels();

    SetupEngine();

    LOG("Successfully created EgoVehicle (%s) rigid body", *GetVehicleType());
}

float AEgoVehicle::GetMaximumSteerAngle() const
{
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());
    check(Vehicle4W != nullptr);
    auto &Wheels = Vehicle4W->Wheels;
    if (Wheels.Num() > 0 && Wheels[0] != nullptr)
    {
        return Wheels[0]->SteerAngle; // front wheel is wheel[0]
    }
    return 60.f;
}

bool AEgoVehicle::IsTwoWheeledVehicle_Implementation()
{
    return bIs2Wheeled;
}

void AEgoVehicle::SetupWheels()
{
    // set up wheels (assume 4w, but if not then try 2w)
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());
    check(Vehicle4W != nullptr);
    check(Vehicle4W->WheelSetups.Num() == 4); // even 2wheeledvehicles have 4 wheels in carla, they just fake it

    /// TODO: ensure the wheels bonename matches the Carla SkeletalMesh
    // Setup the wheels
    if (!IsTwoWheeledVehicle_Implementation()) // 4wheeledvehicle
    {
        LOG("Initializing 4-wheeled-vehicle");
        Vehicle4W->WheelSetups[0].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[0].BoneName = FName("Wheel_Front_Left");

        Vehicle4W->WheelSetups[1].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[1].BoneName = FName("Wheel_Front_Right");

        Vehicle4W->WheelSetups[2].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[2].BoneName = FName("Wheel_Rear_Left");
        Vehicle4W->WheelSetups[2].bDisableSteering = true;

        Vehicle4W->WheelSetups[3].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[3].BoneName = FName("Wheel_Rear_Right");
        Vehicle4W->WheelSetups[3].bDisableSteering = true;
    }
    else // 2wheeledVehicle
    {
        LOG("Initializing 2-wheeled-vehicle"); // the wheels are named differently for some reason
        Vehicle4W->WheelSetups[0].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[0].BoneName = FName("Wheel_F_L");

        Vehicle4W->WheelSetups[1].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[1].BoneName = FName("Wheel_F_R");

        Vehicle4W->WheelSetups[2].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[2].BoneName = FName("Wheel_R_L");
        Vehicle4W->WheelSetups[2].bDisableSteering = true;

        Vehicle4W->WheelSetups[3].WheelClass = UVehicleWheel::StaticClass();
        Vehicle4W->WheelSetups[3].BoneName = FName("Wheel_R_R");
        Vehicle4W->WheelSetups[3].bDisableSteering = true;
    }

    // Adjust the tire loading
    Vehicle4W->MinNormalizedTireLoad = 0.0f;
    Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
    Vehicle4W->MaxNormalizedTireLoad = 2.0f;
    Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;
}

void AEgoVehicle::SetupEngine()
{
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());
    check(Vehicle4W != nullptr);

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
    // add safety guarantee for initializing the TireConfig variable
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());
    check(Vehicle4W != nullptr);
    check(Vehicle4W->Wheels.Num() == WheelsFrictionScale.Num());

    for (int32 i = 0; i < Vehicle4W->Wheels.Num(); ++i)
    {
        check(Vehicle4W->Wheels[i] != nullptr);
        if (Vehicle4W->Wheels[i]->TireConfig == nullptr)
        {
            Vehicle4W->Wheels[i]->TireConfig = NewObject<UTireConfig>();
        }
        Vehicle4W->Wheels[i]->TireConfig->SetFrictionScale(WheelsFrictionScale[i]);
    }
}