#pragma once

#include "Camera/CameraComponent.h" // UCameraComponent
#include "EgoVehicle.h"             // AEgoVehicle
#include "Engine/Scene.h"           // FPostProcessSettings
#include "GameFramework/Pawn.h"     // CreatePlayerInputComponent

// #define USE_LOGITECH_PLUGIN true // handled in .Build.cs file

#ifndef _WIN32
// can only use LogitechWheel plugin on Windows! :(
#undef USE_LOGITECH_PLUGIN
#define USE_LOGITECH_PLUGIN false
#endif

#if USE_LOGITECH_PLUGIN
#include "LogitechSteeringWheelLib.h" // LogitechWheel plugin for hardware integration & force feedback
#endif

#include "DReyeVRPawn.generated.h"

UCLASS()
class ADReyeVRPawn : public APawn
{
    GENERATED_BODY()

  public:
    ADReyeVRPawn(const FObjectInitializer &ObjectInitializer);

    virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;
    virtual void Tick(float DeltaSeconds) override;

    void AttachToEgoVehicle(AEgoVehicle *Vehicle, UWorld *World, APlayerController *PlayerIn)
    {
        EgoVehicle = Vehicle;
        this->Player = PlayerIn;
        ensure(this->Player != nullptr);

        EgoVehicle->SetPawn(this);
        FirstPersonCam->RegisterComponentWithWorld(World);
    }

    APlayerController *GetPlayer()
    {
        return Player;
    }

    UCameraComponent *GetCamera()
    {
        return FirstPersonCam;
    }

    const UCameraComponent *GetCamera() const
    {
        return FirstPersonCam;
    }

    bool GetIsLogiConnected() const
    {
        return bIsLogiConnected;
    }

  protected:
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

  private:
    UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent *FirstPersonCam;

    class AEgoVehicle *EgoVehicle;

    APlayerController *Player = nullptr;

    float FieldOfView = 90.f; // in degrees

    FPostProcessSettings PostProcessingInit();
    void ReadConfigVariables();

    // inputs
    /// TODO: refactor so they are only defined here, not in EgoVehicle!
    void MouseLookUp(const float mY_Input);
    void MouseTurn(const float mX_Input);
    bool InvertMouseY;
    float ScaleMouseY;
    float ScaleMouseX;

    // keyboard mechanisms to access Axis vehicle control (steering, throttle, brake)
    void SetBrakeKbd(const float in);
    void SetSteeringKbd(const float in);
    void SetThrottleKbd(const float in);

    void PressReverse();
    void ReleaseReverse();

    void PressTurnSignalL();
    void ReleaseTurnSignalL();

    void PressTurnSignalR();
    void ReleaseTurnSignalR();

    void PressHandbrake();
    void ReleaseHandbrake();

    void CameraFwd();
    void CameraBack();
    void CameraLeft();
    void CameraRight();
    void CameraUp();
    void CameraDown();

    UWorld *World = nullptr;

    // default logi plugin behaviour is to set things to 0.5 for some reason
    // "Pedals will output a value of 0.5 until the wheel/pedals receive any kind of input."
    // https://github.com/HARPLab/LogitechWheelPlugin
    bool bPedalsDefaulting = true;

    // logi

    void InitLogiWheel();
    void TickLogiWheel();
    void DestroyLogiWheel(bool DestroyModule);
    bool bLogLogitechWheel = false;
    int WheelDeviceIdx = 0; // usually leaving as 0 is fine, only use 1 if 0 is taken
#if USE_LOGITECH_PLUGIN
    struct DIJOYSTATE2 *Old = nullptr; // global "old" struct for the last state
    void LogLogitechPluginStruct(const struct DIJOYSTATE2 *Now);
    void LogitechWheelUpdate();      // for logitech wheel integration
    void ApplyForceFeedback() const; // for logitech wheel integration
#endif
    bool bIsLogiConnected = false; // check if Logi device is connected (on BeginPlay)
};
