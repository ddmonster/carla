#pragma once

#include <fstream>
#include <vector>
// DReyeVR include
#include "Carla/Sensor/DReyeVRData.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is a custom class to add arbitrary actors to the recorder without affecting compatibility of old recordings //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)
struct DReyeVRCustomActorRecorder
{
    DReyeVRCustomActorRecorder() = default;
    DReyeVRCustomActorRecorder(const DReyeVR::CustomActorData *DataIn)
    {
        Data = (*DataIn);
    }
    DReyeVR::CustomActorData Data;
    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    std::string Print() const;
};

#pragma pack(pop)
class DReyeVRCustomActorRecorders
{

  public:
    void Add(const DReyeVRCustomActorRecorder &NewData);
    void Clear(void);
    void Write(std::ofstream &OutFile);

  private:
    // using a vector as a queue that holds everything, gets written and flushed on every tick
    std::vector<DReyeVRCustomActorRecorder> AllData;
};
