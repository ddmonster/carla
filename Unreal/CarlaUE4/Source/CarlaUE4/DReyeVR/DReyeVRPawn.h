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

    void AttachToEgoVehicle(AEgoVehicle *Vehicle, UWorld *World)
    {
        EgoVehicle = Vehicle;
        EgoVehicle->AssignFirstPersonCam(this);
        FirstPersonCam->RegisterComponentWithWorld(World);
    }

    UCameraComponent *GetFirstPersonCam()
    {
        return FirstPersonCam;
    }

  protected:
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;


  private:
    UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent *FirstPersonCam;

    class AEgoVehicle *EgoVehicle;

    float FieldOfView = 90.f; // in degrees

    FPostProcessSettings PostProcessingInit();
    void ReadConfigVariables();

    // inputs
    /// TODO: refactor so they are only defined here, not in EgoVehicle!
    void MouseLookUp(const float mY_Input);
    void MouseTurn(const float mX_Input);

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
