#include "DReyeVRPawn.h"

ADReyeVRPawn::ADReyeVRPawn(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    auto *RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DReyeVR_RootComponent"));
    SetRootComponent(RootComponent);

    // auto possess player 0
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    // Create a camera and attach to root component
    ReadConfigValue("EgoVehicle", "FieldOfView", FieldOfView);

    FirstPersonCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCam"));
    FirstPersonCam->PostProcessSettings = PostProcessingInit();
    FirstPersonCam->bUsePawnControlRotation = false; // free for VR movement
    FirstPersonCam->bLockToHmd = true;               // lock orientation and position to HMD
    FirstPersonCam->FieldOfView = FieldOfView;       // editable
    FirstPersonCam->SetupAttachment(RootComponent);
}

void ADReyeVRPawn::BeginPlay()
{
    Super::BeginPlay();
}

void ADReyeVRPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

FPostProcessSettings ADReyeVRPawn::PostProcessingInit()
{
    // modifying from here: https://docs.unrealengine.com/4.27/en-US/API/Runtime/Engine/Engine/FPostProcessSettings/
    FPostProcessSettings PP;
    PP.bOverride_VignetteIntensity = true;
    PP.VignetteIntensity = 0.f;

    PP.ScreenPercentage = 100.f;

    PP.bOverride_BloomIntensity = true;
    PP.BloomIntensity = 0.f;

    PP.bOverride_SceneFringeIntensity = true;
    PP.SceneFringeIntensity = 0.f;

    PP.bOverride_LensFlareIntensity = true;
    PP.LensFlareIntensity = 0.f;

    PP.bOverride_GrainIntensity = true;
    PP.GrainIntensity = 0.f;
    // PP.MotionBlurAmount = 0.f;
    return PP;
}

void ADReyeVRPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    /// NOTE: an Action is a digital input, an Axis is an analog input
    // steering and throttle analog inputs (axes)
    PlayerInputComponent->BindAxis("Steer_DReyeVR", this, &ADReyeVRPawn::SetSteeringKbd);
    PlayerInputComponent->BindAxis("Throttle_DReyeVR", this, &ADReyeVRPawn::SetThrottleKbd);
    PlayerInputComponent->BindAxis("Brake_DReyeVR", this, &ADReyeVRPawn::SetBrakeKbd);
    // // button actions (press & release)
    PlayerInputComponent->BindAction("ToggleReverse_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::PressReverse);
    PlayerInputComponent->BindAction("TurnSignalRight_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::PressTurnSignalR);
    PlayerInputComponent->BindAction("TurnSignalLeft_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::PressTurnSignalL);
    PlayerInputComponent->BindAction("ToggleReverse_DReyeVR", IE_Released, this, &ADReyeVRPawn::ReleaseReverse);
    PlayerInputComponent->BindAction("TurnSignalRight_DReyeVR", IE_Released, this, &ADReyeVRPawn::ReleaseTurnSignalR);
    PlayerInputComponent->BindAction("TurnSignalLeft_DReyeVR", IE_Released, this, &ADReyeVRPawn::ReleaseTurnSignalL);
    PlayerInputComponent->BindAction("HoldHandbrake_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::PressHandbrake);
    PlayerInputComponent->BindAction("HoldHandbrake_DReyeVR", IE_Released, this, &ADReyeVRPawn::ReleaseHandbrake);
    // clean/empty room experiment
    // PlayerInputComponent->BindAction("ToggleCleanRoom_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::ToggleCleanRoom);
    /// Mouse X and Y input for looking up and turning
    PlayerInputComponent->BindAxis("MouseLookUp_DReyeVR", this, &ADReyeVRPawn::MouseLookUp);
    PlayerInputComponent->BindAxis("MouseTurn_DReyeVR", this, &ADReyeVRPawn::MouseTurn);
    // Camera position adjustments
    PlayerInputComponent->BindAction("CameraFwd_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::CameraFwd);
    PlayerInputComponent->BindAction("CameraBack_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::CameraBack);
    PlayerInputComponent->BindAction("CameraLeft_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::CameraLeft);
    PlayerInputComponent->BindAction("CameraRight_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::CameraRight);
    PlayerInputComponent->BindAction("CameraUp_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::CameraUp);
    PlayerInputComponent->BindAction("CameraDown_DReyeVR", IE_Pressed, this, &ADReyeVRPawn::CameraDown);
}
/// TODO: REFACTOR THIS
#define CHECK_EGO_VEHICLE(FUNCTION)                                                                                    \
    if (EgoVehicle)                                                                                                    \
        FUNCTION;                                                                                                      \
    else                                                                                                               \
        UE_LOG(LogTemp, Warning, TEXT("No egovehicle!"));

void ADReyeVRPawn::SetThrottleKbd(const float ThrottleInput)
{
    if (ThrottleInput != 0)
    {
        CHECK_EGO_VEHICLE(EgoVehicle->SetThrottleKbd(ThrottleInput))
    }
}

void ADReyeVRPawn::SetBrakeKbd(const float BrakeInput)
{
    if (BrakeInput != 0)
    {
        CHECK_EGO_VEHICLE(EgoVehicle->SetBrakeKbd(BrakeInput))
    }
}

void ADReyeVRPawn::SetSteeringKbd(const float SteeringInput)
{
    if (SteeringInput != 0)
    {
        CHECK_EGO_VEHICLE(EgoVehicle->SetSteeringKbd(SteeringInput))
    }
    else
    {
        // so the steering wheel does go to 0 when letting go
        ensure(EgoVehicle != nullptr);
        EgoVehicle->VehicleInputs.Steering = 0;
    }
}

void ADReyeVRPawn::MouseLookUp(const float mY_Input)
{
    CHECK_EGO_VEHICLE(EgoVehicle->MouseLookUp(mY_Input))
}

void ADReyeVRPawn::MouseTurn(const float mX_Input)
{
    CHECK_EGO_VEHICLE(EgoVehicle->MouseTurn(mX_Input))
}

void ADReyeVRPawn::PressReverse()
{
    CHECK_EGO_VEHICLE(EgoVehicle->PressReverse())
}

void ADReyeVRPawn::ReleaseReverse()
{
    CHECK_EGO_VEHICLE(EgoVehicle->ReleaseReverse())
}

void ADReyeVRPawn::PressTurnSignalL()
{
    CHECK_EGO_VEHICLE(EgoVehicle->PressTurnSignalL())
}

void ADReyeVRPawn::ReleaseTurnSignalL()
{
    CHECK_EGO_VEHICLE(EgoVehicle->ReleaseTurnSignalL())
}

void ADReyeVRPawn::PressTurnSignalR()
{
    CHECK_EGO_VEHICLE(EgoVehicle->PressTurnSignalR())
}

void ADReyeVRPawn::ReleaseTurnSignalR()
{
    CHECK_EGO_VEHICLE(EgoVehicle->ReleaseTurnSignalR())
}

void ADReyeVRPawn::PressHandbrake()
{
    CHECK_EGO_VEHICLE(EgoVehicle->PressHandbrake())
}

void ADReyeVRPawn::ReleaseHandbrake()
{
    CHECK_EGO_VEHICLE(EgoVehicle->ReleaseHandbrake())
}

void ADReyeVRPawn::CameraFwd()
{
    CHECK_EGO_VEHICLE(EgoVehicle->CameraFwd())
}

void ADReyeVRPawn::CameraBack()
{
    CHECK_EGO_VEHICLE(EgoVehicle->CameraBack())
}

void ADReyeVRPawn::CameraLeft()
{
    CHECK_EGO_VEHICLE(EgoVehicle->CameraLeft())
}

void ADReyeVRPawn::CameraRight()
{
    CHECK_EGO_VEHICLE(EgoVehicle->CameraRight())
}

void ADReyeVRPawn::CameraUp()
{
    CHECK_EGO_VEHICLE(EgoVehicle->CameraUp())
}

void ADReyeVRPawn::CameraDown()
{
    CHECK_EGO_VEHICLE(EgoVehicle->CameraDown())
}
