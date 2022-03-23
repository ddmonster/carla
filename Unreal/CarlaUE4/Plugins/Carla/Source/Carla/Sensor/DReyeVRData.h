#pragma once
#ifndef DREYEVR_SENSOR_DATA
#define DREYEVR_SENSOR_DATA

#include "Carla/Recorder/CarlaRecorderHelpers.h" // WriteValue, WriteFVector, WriteFString, ...
#include <chrono>                                // timing threads
#include <cstdint>                               // int64_t
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace DReyeVR
{
struct EyeData
{
    FVector GazeDir = FVector::ZeroVector;
    FVector GazeOrigin = FVector::ZeroVector;
    bool GazeValid = false;
    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
};

struct CombinedEyeData : EyeData
{
    float Vergence = 0.f; // in cm (default UE4 units)
    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
};

struct SingleEyeData : EyeData
{
    float EyeOpenness = 0.f;
    bool EyeOpennessValid = false;
    float PupilDiameter = 0.f;
    FVector2D PupilPosition = FVector2D::ZeroVector;
    bool PupilPositionValid = false;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
};

struct EgoVariables
{
    // World coordinate Ego vehicle location & rotation
    FVector VehicleLocation = FVector::ZeroVector;
    FRotator VehicleRotation = FRotator::ZeroRotator;
    // Relative Camera position and orientation (for HMD offsets)
    FVector CameraLocation = FVector::ZeroVector;
    FRotator CameraRotation = FRotator::ZeroRotator;
    // Absolute Camera position and orientation (includes vehicle & HMD offset)
    FVector CameraLocationAbs = FVector::ZeroVector;
    FRotator CameraRotationAbs = FRotator::ZeroRotator;
    // Ego variables
    float Velocity = 0.f; // note this is in cm/s (default UE4 units)
    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
};

struct UserInputs
{
    // User inputs
    float Throttle = 0.f;
    float Steering = 0.f;
    float Brake = 0.f;
    bool ToggledReverse = false;
    bool TurnSignalLeft = false;
    bool TurnSignalRight = false;
    bool HoldHandbrake = false;
    // Add more inputs here!

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
};

struct FocusInfo
{
    // substitute for SRanipal FFocusInfo in SRanipal_Eyes_Enums.h
    TWeakObjectPtr<class AActor> Actor;
    FVector HitPoint; // in world space (absolute location)
    FVector Normal;
    FString ActorNameTag = "None"; // Tag of the actor being focused on
    float Distance;
    bool bDidHit;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
};

struct EyeTracker
{
    int64_t TimestampDevice = 0; // timestamp from the eye tracker device (with its own clock)
    int64_t FrameSequence = 0;   // "Frame sequence" of SRanipal or just the tick frame in UE4
    CombinedEyeData Combined;
    SingleEyeData Left;
    SingleEyeData Right;
    FVector2D ProjectedCoords;
    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
};

enum class Gaze
{
    COMBINED, // default for functions
    RIGHT,
    LEFT,
};

enum class Eye
{
    RIGHT,
    LEFT,
};

class AggregateData // all DReyeVR sensor data is held here
{
  public:
    AggregateData() = default;
    /////////////////////////:GETTERS://////////////////////////////

    int64_t GetTimestampCarla() const
    {
        return TimestampCarlaUE4;
    }
    int64_t GetTimestampDevice() const
    {
        return EyeTrackerData.TimestampDevice;
    }
    int64_t GetFrameSequence() const
    {
        return EyeTrackerData.FrameSequence;
    }
    float GetGazeVergence() const
    {
        return EyeTrackerData.Combined.Vergence; // in cm (default UE4 units)
    }
    const FVector &GetGazeDir(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        switch (Index)
        {
        case DReyeVR::Gaze::LEFT:
            return EyeTrackerData.Left.GazeDir;
        case DReyeVR::Gaze::RIGHT:
            return EyeTrackerData.Right.GazeDir;
        case DReyeVR::Gaze::COMBINED:
            return EyeTrackerData.Combined.GazeDir;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Combined.GazeDir;
        }
    }
    const FVector &GetGazeOrigin(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        switch (Index)
        {
        case DReyeVR::Gaze::LEFT:
            return EyeTrackerData.Left.GazeOrigin;
        case DReyeVR::Gaze::RIGHT:
            return EyeTrackerData.Right.GazeOrigin;
        case DReyeVR::Gaze::COMBINED:
            return EyeTrackerData.Combined.GazeOrigin;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Combined.GazeOrigin;
        }
    }
    bool GetGazeValidity(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        switch (Index)
        {
        case DReyeVR::Gaze::LEFT:
            return EyeTrackerData.Left.GazeValid;
        case DReyeVR::Gaze::RIGHT:
            return EyeTrackerData.Right.GazeValid;
        case DReyeVR::Gaze::COMBINED:
            return EyeTrackerData.Combined.GazeValid;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Combined.GazeValid;
        }
    }
    float GetEyeOpenness(DReyeVR::Eye Index) const // returns eye openness as a percentage [0,1]
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.EyeOpenness;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.EyeOpenness;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Right.EyeOpenness;
        }
    }
    bool GetEyeOpennessValidity(DReyeVR::Eye Index) const
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.EyeOpennessValid;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.EyeOpennessValid;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Right.EyeOpennessValid;
        }
    }
    float GetPupilDiameter(DReyeVR::Eye Index) const // returns diameter in mm
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.PupilDiameter;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.PupilDiameter;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Right.PupilDiameter;
        }
    }
    const FVector2D &GetPupilPosition(DReyeVR::Eye Index) const
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.PupilPosition;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.PupilPosition;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Right.PupilPosition;
        }
    }
    bool GetPupilPositionValidity(DReyeVR::Eye Index) const
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.PupilPositionValid;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.PupilPositionValid;
        default: // need a default case for MSVC >:(
            return EyeTrackerData.Right.PupilPositionValid;
        }
    }
    const FVector2D &GetProjectedReticleCoords() const
    {
        return EyeTrackerData.ProjectedCoords;
    }

    // from EgoVars
    const FVector &GetCameraLocation() const
    {
        return EgoVars.CameraLocation;
    }
    const FRotator &GetCameraRotation() const
    {
        return EgoVars.CameraRotation;
    }
    const FVector &GetCameraLocationAbs() const
    {
        return EgoVars.CameraLocationAbs;
    }
    const FRotator &GetCameraRotationAbs() const
    {
        return EgoVars.CameraRotationAbs;
    }
    float GetVehicleVelocity() const
    {
        return EgoVars.Velocity; // returns ego velocity in cm/s
    }
    const FVector &GetVehicleLocation() const
    {
        return EgoVars.VehicleLocation;
    }
    const FRotator &GetVehicleRotation() const
    {
        return EgoVars.VehicleRotation;
    }
    // focus
    const FString &GetFocusActorName() const
    {
        return FocusData.ActorNameTag;
    }
    const FVector &GetFocusActorPoint() const
    {
        return FocusData.HitPoint;
    }
    float GetFocusActorDistance() const
    {
        return FocusData.Distance;
    }
    const DReyeVR::UserInputs &GetUserInputs() const
    {
        return Inputs;
    }
    ////////////////////:SETTERS://////////////////////

    void UpdateCamera(const FVector &NewCameraLoc, const FRotator &NewCameraRot)
    {
        EgoVars.CameraLocation = NewCameraLoc;
        EgoVars.CameraRotation = NewCameraRot;
    }

    void UpdateCameraAbs(const FVector &NewCameraLocAbs, const FRotator &NewCameraRotAbs)
    {
        EgoVars.CameraLocationAbs = NewCameraLocAbs;
        EgoVars.CameraRotationAbs = NewCameraRotAbs;
    }

    void UpdateVehicle(const FVector &NewVehicleLoc, const FRotator &NewVehicleRot)
    {
        EgoVars.VehicleLocation = NewVehicleLoc;
        EgoVars.VehicleRotation = NewVehicleRot;
    }

    void Update(int64_t NewTimestamp, const struct EyeTracker &NewEyeData, const struct EgoVariables &NewEgoVars,
                const struct FocusInfo &NewFocus, const struct UserInputs &NewInputs)
    {
        TimestampCarlaUE4 = NewTimestamp;
        EyeTrackerData = NewEyeData;
        EgoVars = NewEgoVars;
        FocusData = NewFocus;
        Inputs = NewInputs;
    }

    ////////////////////:SERIALIZATION://////////////////////
    void Read(std::ifstream &InFile)
    {
        /// CAUTION: make sure the order of writes/reads is the same
        ReadValue<int64_t>(InFile, TimestampCarlaUE4);
        EgoVars.Read(InFile);
        EyeTrackerData.Read(InFile);
        FocusData.Read(InFile);
        Inputs.Read(InFile);
    }

    void Write(std::ofstream &OutFile) const
    {
        /// CAUTION: make sure the order of writes/reads is the same
        WriteValue<int64_t>(OutFile, GetTimestampCarla());
        EgoVars.Write(OutFile);
        EyeTrackerData.Write(OutFile);
        FocusData.Write(OutFile);
        Inputs.Write(OutFile);
    }

    FString ToString() const
    {
        FString print;
        print += FString::Printf(TEXT("[DReyeVR]TimestampCarla:%ld,\n"), long(TimestampCarlaUE4));
        print += FString::Printf(TEXT("[DReyeVR]EyeTracker:%s,\n"), *EyeTrackerData.ToString());
        print += FString::Printf(TEXT("[DReyeVR]FocusInfo:%s,\n"), *FocusData.ToString());
        print += FString::Printf(TEXT("[DReyeVR]EgoVariables:%s,\n"), *EgoVars.ToString());
        print += FString::Printf(TEXT("[DReyeVR]UserInputs:%s,\n"), *Inputs.ToString());
        return print;
    }

    std::string GetUniqueName() const
    {
        return "DReyeVRSensorAggregateData";
    }

  private:
    int64_t TimestampCarlaUE4; // Carla Timestamp (EgoSensor Tick() event) in milliseconds
    struct EyeTracker EyeTrackerData;
    struct EgoVariables EgoVars;
    struct FocusInfo FocusData;
    struct UserInputs Inputs;
};

class CustomActorData
{
  public:
    FString Name; // unique actor name of this actor
    FVector Location;
    FRotator Rotation;
    FVector Scale3D;
    FString Other; // any other data deemed necessary to record
    char TypeId;
    enum class Types : uint8_t
    {
        SPHERE = 0,
        CROSS
    };

    CustomActorData() = default;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    FString ToString() const;
    std::string GetUniqueName() const;
};

}; // namespace DReyeVR

#endif