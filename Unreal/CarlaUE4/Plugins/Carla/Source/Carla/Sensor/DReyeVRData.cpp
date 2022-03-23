#include "DReyeVRData.h"

#include <string>
#include <unordered_map>

namespace DReyeVR
{

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