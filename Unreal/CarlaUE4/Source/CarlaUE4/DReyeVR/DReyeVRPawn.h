#pragma once

#include "Camera/CameraComponent.h" // UCameraComponent
#include "EgoVehicle.h"             // AEgoVehicle
#include "Engine/Scene.h"           // FPostProcessSettings
#include "GameFramework/Pawn.h"     // CreatePlayerInputComponent

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

  private:
    UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent *FirstPersonCam;

    class AEgoVehicle *EgoVehicle;

    float FieldOfView = 90.f; // in degrees

    FPostProcessSettings PostProcessingInit();

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
};
