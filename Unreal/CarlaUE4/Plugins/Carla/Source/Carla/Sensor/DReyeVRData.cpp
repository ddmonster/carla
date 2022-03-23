#include "DReyeVRData.h"

#include <string>
#include <unordered_map>

namespace DReyeVR
{

/// ========================================== ///
/// ----------------:EYEDATA:----------------- ///
/// ========================================== ///

void EyeData::Read(std::ifstream &InFile)
{
    ReadFVector(InFile, GazeDir);
    ReadFVector(InFile, GazeOrigin);
    ReadValue<bool>(InFile, GazeValid);
}

void EyeData::Write(std::ofstream &OutFile) const
{
    WriteFVector(OutFile, GazeDir);
    WriteFVector(OutFile, GazeOrigin);
    WriteValue<bool>(OutFile, GazeValid);
}

FString EyeData::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("GazeDir:%s,"), *GazeDir.ToString());
    Print += FString::Printf(TEXT("GazeOrigin:%s,"), *GazeOrigin.ToString());
    Print += FString::Printf(TEXT("GazeValid:%d,"), GazeValid);
    return Print;
}

/// ========================================== ///
/// ------------:COMBINEDEYEDATA:------------- ///
/// ========================================== ///

void CombinedEyeData::Read(std::ifstream &InFile)
{
    EyeData::Read(InFile);
    ReadValue<float>(InFile, Vergence);
}

void CombinedEyeData::Write(std::ofstream &OutFile) const
{
    EyeData::Write(OutFile);
    WriteValue<float>(OutFile, Vergence);
}

FString CombinedEyeData::ToString() const
{
    FString Print = EyeData::ToString();
    Print += FString::Printf(TEXT("GazeVergence:%.4f,"), Vergence);
    return Print;
}

/// ========================================== ///
/// -------------:SINGLEEYEDATA:-------------- ///
/// ========================================== ///

void SingleEyeData::Read(std::ifstream &InFile)
{
    EyeData::Read(InFile);
    ReadValue<float>(InFile, EyeOpenness);
    ReadValue<bool>(InFile, EyeOpennessValid);
    ReadValue<float>(InFile, PupilDiameter);
    ReadFVector2D(InFile, PupilPosition);
    ReadValue<bool>(InFile, PupilPositionValid);
}

void SingleEyeData::Write(std::ofstream &OutFile) const
{
    EyeData::Write(OutFile);
    WriteValue<float>(OutFile, EyeOpenness);
    WriteValue<bool>(OutFile, EyeOpennessValid);
    WriteValue<float>(OutFile, PupilDiameter);
    WriteFVector2D(OutFile, PupilPosition);
    WriteValue<bool>(OutFile, PupilPositionValid);
}

FString SingleEyeData::ToString() const
{
    FString Print = EyeData::ToString();
    Print += FString::Printf(TEXT("EyeOpenness:%.4f,"), EyeOpenness);
    Print += FString::Printf(TEXT("EyeOpennessValid:%d,"), EyeOpennessValid);
    Print += FString::Printf(TEXT("PupilDiameter:%.4f,"), PupilDiameter);
    Print += FString::Printf(TEXT("PupilPosition:%s,"), *PupilPosition.ToString());
    Print += FString::Printf(TEXT("PupilPositionValid:%d,"), PupilPositionValid);
    return Print;
}

/// ========================================== ///
/// --------------:EGOVARIABLES:-------------- ///
/// ========================================== ///

void EgoVariables::Read(std::ifstream &InFile)
{
    ReadFVector(InFile, CameraLocation);
    ReadFRotator(InFile, CameraRotation);
    ReadFVector(InFile, CameraLocationAbs);
    ReadFRotator(InFile, CameraRotationAbs);
    ReadFVector(InFile, VehicleLocation);
    ReadFRotator(InFile, VehicleRotation);
    ReadValue<float>(InFile, Velocity);
}

void EgoVariables::Write(std::ofstream &OutFile) const
{
    WriteFVector(OutFile, CameraLocation);
    WriteFRotator(OutFile, CameraRotation);
    WriteFVector(OutFile, CameraLocationAbs);
    WriteFRotator(OutFile, CameraRotationAbs);
    WriteFVector(OutFile, VehicleLocation);
    WriteFRotator(OutFile, VehicleRotation);
    WriteValue<float>(OutFile, Velocity);
}

FString EgoVariables::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("VehicleLoc:%s,"), *VehicleLocation.ToString());
    Print += FString::Printf(TEXT("VehicleRot:%s,"), *VehicleRotation.ToString());
    Print += FString::Printf(TEXT("VehicleVel:%.4f,"), Velocity);
    Print += FString::Printf(TEXT("CameraLoc:%s,"), *CameraLocation.ToString());
    Print += FString::Printf(TEXT("CameraRot:%s,"), *CameraRotation.ToString());
    Print += FString::Printf(TEXT("CameraLocAbs:%s,"), *CameraLocationAbs.ToString());
    Print += FString::Printf(TEXT("CameraRotAbs:%s,"), *CameraRotationAbs.ToString());
    return Print;
}

/// ========================================== ///
/// --------------:USERINPUTS:---------------- ///
/// ========================================== ///

void UserInputs::Read(std::ifstream &InFile)
{
    ReadValue<float>(InFile, Throttle);
    ReadValue<float>(InFile, Steering);
    ReadValue<float>(InFile, Brake);
    ReadValue<bool>(InFile, ToggledReverse);
    ReadValue<bool>(InFile, TurnSignalLeft);
    ReadValue<bool>(InFile, TurnSignalRight);
    ReadValue<bool>(InFile, HoldHandbrake);
}

void UserInputs::Write(std::ofstream &OutFile) const
{
    WriteValue<float>(OutFile, Throttle);
    WriteValue<float>(OutFile, Steering);
    WriteValue<float>(OutFile, Brake);
    WriteValue<bool>(OutFile, ToggledReverse);
    WriteValue<bool>(OutFile, TurnSignalLeft);
    WriteValue<bool>(OutFile, TurnSignalRight);
    WriteValue<bool>(OutFile, HoldHandbrake);
}

FString UserInputs::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("Throttle:%.4f,"), Throttle);
    Print += FString::Printf(TEXT("Steering:%.4f,"), Steering);
    Print += FString::Printf(TEXT("Brake:%.4f,"), Brake);
    Print += FString::Printf(TEXT("ToggledReverse:%d,"), ToggledReverse);
    Print += FString::Printf(TEXT("TurnSignalLeft:%d,"), TurnSignalLeft);
    Print += FString::Printf(TEXT("TurnSignalRight:%d,"), TurnSignalRight);
    Print += FString::Printf(TEXT("HoldHandbrake:%d,"), HoldHandbrake);
    return Print;
}

/// ========================================== ///
/// ---------------:FOCUSINFO:---------------- ///
/// ========================================== ///

void FocusInfo::Read(std::ifstream &InFile)
{
    ReadFString(InFile, ActorNameTag);
    ReadValue<bool>(InFile, bDidHit);
    ReadFVector(InFile, HitPoint);
    ReadFVector(InFile, Normal);
    ReadValue<float>(InFile, Distance);
}
void FocusInfo::Write(std::ofstream &OutFile) const
{
    WriteFString(OutFile, ActorNameTag);
    WriteValue<bool>(OutFile, bDidHit);
    WriteFVector(OutFile, HitPoint);
    WriteFVector(OutFile, Normal);
    WriteValue<float>(OutFile, Distance);
}
FString FocusInfo::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("Hit:%d,"), bDidHit);
    Print += FString::Printf(TEXT("Distance:%.4f,"), Distance);
    Print += FString::Printf(TEXT("HitPoint:%s,"), *HitPoint.ToString());
    Print += FString::Printf(TEXT("HitNormal:%s,"), *Normal.ToString());
    Print += FString::Printf(TEXT("ActorName:%s,"), *ActorNameTag);
    return Print;
}

/// ========================================== ///
/// ---------------:EYETRACKER:--------------- ///
/// ========================================== ///

void EyeTracker::Read(std::ifstream &InFile)
{
    ReadValue<int64_t>(InFile, TimestampDevice);
    ReadValue<int64_t>(InFile, FrameSequence);
    Combined.Read(InFile);
    Left.Read(InFile);
    Right.Read(InFile);
    ReadFVector2D(InFile, ProjectedCoords);
}

void EyeTracker::Write(std::ofstream &OutFile) const
{
    WriteValue<int64_t>(OutFile, TimestampDevice);
    WriteValue<int64_t>(OutFile, FrameSequence);
    Combined.Write(OutFile);
    Left.Write(OutFile);
    Right.Write(OutFile);
    WriteFVector2D(OutFile, ProjectedCoords);
}

FString EyeTracker::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("TimestampDevice:%ld,"), long(TimestampDevice));
    Print += FString::Printf(TEXT("FrameSequence:%ld,"), long(FrameSequence));
    Print += FString::Printf(TEXT("COMBINED:{%s},"), *Combined.ToString());
    Print += FString::Printf(TEXT("LEFT:{%s},"), *Left.ToString());
    Print += FString::Printf(TEXT("RIGHT:{%s},"), *Right.ToString());
    Print += FString::Printf(TEXT("ReticleCoords:%s,"), *ProjectedCoords.ToString());
    return Print;
}

/// ========================================== ///
/// ------------:CUSTOMACTORDATA:------------- ///
/// ========================================== ///

void CustomActorData::Read(std::ifstream &InFile)
{
    ReadValue<char>(InFile, TypeId);
    ReadFVector(InFile, Location);
    ReadFRotator(InFile, Rotation);
    ReadFVector(InFile, Scale3D);
    ReadFString(InFile, Other);
    ReadFString(InFile, Name);
}

void CustomActorData::Write(std::ofstream &OutFile) const
{
    WriteValue<char>(OutFile, static_cast<char>(TypeId));
    WriteFVector(OutFile, Location);
    WriteFRotator(OutFile, Rotation);
    WriteFVector(OutFile, Scale3D);
    WriteFString(OutFile, Other);
    WriteFString(OutFile, Name);
}

FString CustomActorData::ToString() const
{
    FString Print = "";
    Print += FString::Printf(TEXT("Type:%d,"), int(TypeId));
    Print += FString::Printf(TEXT("Name:%s,"), *Name);
    Print += FString::Printf(TEXT("Location:%s,"), *Location.ToString());
    Print += FString::Printf(TEXT("Rotation:%s,"), *Rotation.ToString());
    Print += FString::Printf(TEXT("Scale3D:%s,"), *Scale3D.ToString());
    Print += FString::Printf(TEXT("Other:%s,"), *Other);
    return Print;
}

std::string CustomActorData::GetUniqueName() const
{
    return TCHAR_TO_UTF8(*Name);
}

}; // namespace DReyeVR