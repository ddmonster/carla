#include "DummyWalkers.h"
#include "Carla/Actor/ActorDefinition.h" // FActorDefinition
#include "Carla/Actor/CarlaActor.h"      // FCarlaActor
#include "Carla/Game/CarlaStatics.h"     // GetRecorder, GetEpisode
#include "Carla/Walker/WalkerBase.h"     // AWalkerBase
#include "Carla/Walker/WalkerControl.h"  // FWalkerControl
#include "DReyeVRUtils.h"                // bIsWalkable

DummyWalkers::DummyWalkers()
{
    ReadConfigValue("DummyWalkers", "NumberOfInitialWalkers", NumWalkers);
    ReadConfigValue("DummyWalkers", "RandomSeed", Seed);
    ReadConfigValue("DummyWalkers", "PolicyTickRate", ActorPolicyTickRate);
    ReadConfigValue("DummyWalkers", "Tag", DummyWalkerTag);

    // seed random number generator (one per random sequence)
    SpawnRNG = std::mt19937(Seed);
    DescrRNG = std::mt19937(Seed);
    PhysicsRNG = std::mt19937(Seed);
}

void DummyWalkers::Setup(UWorld *World)
{
    // this is a very expensive function to spawn all the initial walkers
    // a) find all possible walker spawn locations (this is very slow)
    // b) designates what kind of actors (by blueprint definition) to spawn
    // c) goes and spawns all the walkers

    if (NumWalkers == 0) // if we are spawning no actors to begin with, skip this
        return;

    UE_LOG(
        LogTemp, Warning,
        TEXT("NOTE: DummyWalkers C++ spawn is slow and kinda broken. It is recommended to instead spawn your walkers "
             "with PythonAPI and apply the DummyWalker tag: actor.apply_tag(\"DummyWalker\") to get a better effect."));
#if 0 // this also won't add the actors to the CarlaRegistry so they won't be findable anyways!
    // get carla episode/map
    auto Episode = UCarlaStatics::GetCurrentEpisode(World);
    ensure(Episode != nullptr);
    if (Episode == nullptr)
    {
        return;
    }
    ALargeMapManager *LargeMap = UCarlaStatics::GetLargeMapManager(Episode->GetWorld());

    TArray<FVector> SideWalkLocations;
    { // get sidewalk pieces from the world to spawn walkers on

        // sample points around the vehicle spawn point
        TArray<FTransform> SpawnPoints = Episode->GetRecommendedSpawnPoints();
        constexpr float CM_To_M = 100.f; // cm to m
        const float SearchRadius = 10.f; // meters
        const FVector2D SearchRange{CM_To_M * SearchRadius, CM_To_M * SearchRadius};
        const FVector2D StepSize{CM_To_M * 0.5f, CM_To_M * 0.5}; // resolution for step size (smaller => longer)

        const FBox WorldBounds = ALevelBounds::CalculateLevelBounds(World->GetCurrentLevel());
        const float Height = WorldBounds.Max.Z - WorldBounds.Min.Z;
        for (FTransform &FT : SpawnPoints)
        {
            FVector Center = FT.GetLocation();
            for (float YPos = Center.Y - SearchRange.Y; YPos < Center.Y + SearchRange.Y; YPos += StepSize.Y)
            {
                for (float XPos = Center.X - SearchRange.X; XPos < Center.X + SearchRange.X; XPos += StepSize.X)
                {
                    // compute ground actor
                    FVector TopPosition{XPos, YPos, WorldBounds.Max.Z};
                    FHitResult Hit = DownGroundTrace(World, TopPosition, Height);
                    AActor *GroundActor = Hit.Actor.Get(); // dereference weak ptr
                    if (GroundActor != nullptr && bIsWalkable(GroundActor))
                    {
                        SideWalkLocations.Add(Hit.Location); // absolute location of hit
                    }
                }
            }
        }

        if (SideWalkLocations.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No sidewalks found to spawn walkers on!"));
            return;
        }
    }

    // designate walker definitions
    TArray<FActorDefinition> SpawnableWalkers;
    {
        TArray<FActorDefinition> Spawnable = Episode->GetActorDefinitions();
        for (FActorDefinition &F : Spawnable)
        {
            if (F.Id.ToLower().Contains("walker"))
                SpawnableWalkers.Add(F);
        }
        if (SpawnableWalkers.Num() == 0)
        {
            UE_LOG(LogTemp, Log, TEXT("No available walkers definitions to spawn!"));
            return;
        }
    }

    // begin spawning the walkers
    for (size_t i = 0; i < NumWalkers; i++)
    {
        // designate spawn location
        FTransform SpawnTransform;
        {
            size_t RandIdx = RandRange(Unif01(SpawnRNG), 0, SideWalkLocations.Num() - 1);
            FVector Location = SideWalkLocations[RandIdx];
            Location.Z += 100; // to account for the actor height (cm)
            FRotator Rotation = FRotator::ZeroRotator;
            Rotation.Yaw = RandRange(Unif01(SpawnRNG), -180, 180); // random heading
            SpawnTransform = FTransform{Rotation, Location, FVector::OneVector};
        }

        // spawn Walker
        FCarlaActor *Walker = nullptr;
        {
            // track the spawnable walkers
            FActorDescription Description;
            {
                // designate walker type
                size_t RandIdx = RandRange(Unif01(DescrRNG), 0, SpawnableWalkers.Num() - 1);
                FActorDefinition Definition = SpawnableWalkers[RandIdx];
                Description.UId = Definition.UId;
                Description.Id = Definition.Id;
                Description.Class = Definition.Class;
                // Description.Variations = Definition.Variations;
            }

            // spawn the actor
            auto Result = Episode->SpawnActorWithInfo(SpawnTransform, std::move(Description));

            if (Result.Key != EActorSpawnResultStatus::Success)
            {
                UE_LOG(LogTemp, Error, TEXT("Actor not Spawned"));
                continue;
            }

            if (LargeMap)
            {
                LargeMap->OnActorSpawned(*Result.Value);
            }

            Episode->SerializeActor(Result.Value);

            Walker = Result.Value; // assign FCarlaActor
        }
        ensure(Walker != nullptr);
        AActor *WalkerActor = Walker->GetActor();
        ensure(WalkerActor != nullptr);
        Walkers[WalkerActor] = NewWalker(Walker);
    }
#endif
}

bool DummyWalkers::FindWalkers(UWorld *World)
{
    auto Episode = UCarlaStatics::GetCurrentEpisode(World);
    ensure(Episode != nullptr);
    if (Episode == nullptr)
        return false;

    // clear all the walkers to refresh on the next tick
    decltype(Walkers) WalkersTmp; // new batch of walkers
    const FActorRegistry &Registry = Episode->GetActorRegistry();
    for (auto It = Registry.begin(); It != Registry.end(); ++It)
    {
        FCarlaActor *Actor = It.Value().Get();
        if (Actor == nullptr || Actor->IsPendingKill())
            continue; // skip this actor
        AActor *WalkerActor = Actor->GetActor();
        ensure(WalkerActor != nullptr);
        if (Actor->GetActorType() == FCarlaActor::ActorType::Walker && WalkerActor->ActorHasTag(DummyWalkerTag))
        {
            FCarlaActor *ExistingCarlaActor = nullptr;
            if (Walkers.find(WalkerActor) != Walkers.end())
            { // found existing Walker from previous tick
                ExistingCarlaActor = Walkers[WalkerActor].Walker;
            }
            if (ExistingCarlaActor == nullptr || ExistingCarlaActor->IsPendingKill())
                WalkersTmp[WalkerActor] = NewWalker(Actor);
            else // copy from the existing container
                WalkersTmp[WalkerActor] = Walkers[WalkerActor];
        }
    }
    Walkers.clear();
    Walkers = WalkersTmp;

    // const size_t NewSize = Walkers.size();
    // if (NewSize > 0)
    //     UE_LOG(LogTemp, Log, TEXT("Updated Walker list to with %zu walkers"), NewSize);
    return Walkers.size() > 0;
}

void DummyWalkers::Tick(UWorld *World, const float DeltaSeconds)
{
    // loop over all relevent (rendered) walkers and give them some simple
    // (roomba-like) policy to walk around the sidewalks

    if (TimeSinceLastActorRefresh < ActorPolicyTickRate)
    {
        // keep incrementing the time since last refresh as long as the threshold has not been met
        TimeSinceLastActorRefresh += DeltaSeconds;
        return; // don't update the walker's policy
    }
    else
    {
        // reset the time since last actor was refreshed if beyond the tick threshold
        TimeSinceLastActorRefresh = 0.f;
    }

    /// NOTE: since the WalkerStruct tracks the LastLocation and LastRotation since
    // being spawned with NewWalker(), the tickrate (TimeSinceLastActorRefresh) determines
    // how long this data is "stale" for (until the next dummy walker tick).
    bool bFoundWalkers = FindWalkers(World);
    if (bFoundWalkers)
    {
        ensure(Walkers.size() > 0);
    }

    for (auto &KeyValuePair : Walkers)
    {
        // get AActor from FCarlaActor
        class AActor *WalkerActor = KeyValuePair.first;
        if (WalkerActor == nullptr)
        {
            continue;
        }

        WalkerStruct &WS = KeyValuePair.second;
        class FCarlaActor *Walker = WS.Walker;
        if (Walker == nullptr || Walker->IsPendingKill())
        {
            continue;
        }

        // Skip these walkers
        {
            bool SkipWalker = !WalkerActor->WasRecentlyRendered(ActorPolicyTickRate) || // optimization
                              !WalkerActor->ActorHasTag(DummyWalkerTag);                // not part of this policy
            if (SkipWalker)
            {
                // apply empty control (just stay in place)
                auto Response = Walker->ApplyControlToWalker(FWalkerControl{});
                Walker->SetActorState(carla::rpc::ActorState::Dormant); // go to sleep

                continue; // skip computation for this actor
            }
        }

        const FVector &Location = WalkerActor->GetRootComponent()->GetComponentLocation(); // world space
        FRotator Rotation = WalkerActor->GetRootComponent()->GetComponentRotation();       // world space
        // move player forward by Speed meters per second
        const size_t NumChecks = static_cast<size_t>(360 / WS.AngularStep);
        constexpr float PersonHeightMax = 180; // cm (on average)
        FVector NewWalkableLocation;           // where the actor will land if they take the step

        bool Walkable = false;
        int RemainingIters = 10; // upper bound so loop won't go forever
        float Lookahead = 1.f;   // grows to cover more ground if no walkable found

        // compute whether this walker is stuck
        {
            auto AngularVel = [](AActor *Actor) {
                // taken from CarlaActor.cpp (FCarlaActor::GetActorAngularVelocity)
                UPrimitiveComponent *Primitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (Primitive)
                    return Primitive->GetPhysicsAngularVelocityInDegrees();
                return FVector();
            };
            const float VelocityStoppedThreshCm = 10.f;  // if velocity within this (cm/s) consider "stopped"
            const float AngularVelStoppedThreshCm = 3.f; // if angular velocity within this (deg/s) consider "stopped"
            if (WalkerActor->GetVelocity().Size() < VelocityStoppedThreshCm &&
                AngularVel(WalkerActor).Size() < AngularVelStoppedThreshCm)
                WS.TimeInSamePlace += ActorPolicyTickRate; // its been at least this long since last check
            else
                WS.TimeInSamePlace = 0.f;
        }
        WS.TimeSinceStuck += ActorPolicyTickRate;
        bool bIsStuck = (WS.TimeInSamePlace > 5.f * ActorPolicyTickRate); // stuck => hasn't moved in this long
        if (bIsStuck)
        {
            WS.TimeSinceStuck = 0.f;
        }

        // compute whether this walker should rotate
        while ((Walkable == false && RemainingIters > 0) || bIsStuck)
        {
            for (int i = 0; i < NumChecks && !Walkable; i++)
            {
                const FVector PossibleNewLocation =
                    Location + Lookahead * WS.Speed * Rotation.RotateVector(FVector::ForwardVector);
                { // compute if ground in front of Walker is walkable
                    FHitResult Hit = DownGroundTrace(World, PossibleNewLocation, PersonHeightMax, {WalkerActor});
                    class AActor *GroundActor = nullptr;
                    GroundActor = Hit.Actor.Get(); // dereference weak ptr
                    Walkable = (GroundActor != nullptr && bIsWalkable(GroundActor));
                    NewWalkableLocation = Hit.Location;
                }
                if (!Walkable || bIsStuck)
                { // begin rotation step to see if the next AngularStep is walkable
                    // rotate CW once then CCW once then CW once then CCW, and so on...
                    float sign = (i % 2 == 0) ? 1.f : -1.f;  // 1 => CW, -1 => CCW
                    if (bIsStuck || WS.TimeSinceStuck < 1.f) // stuck or was stuck recently
                    {
                        sign = 1.f;       // make it so recovery from being stuck is always CW
                        bIsStuck = false; // to end the loop! (after a slight rotation)
                    }
                    Rotation.Yaw += sign * (i + 1) * WS.AngularStep;
                }
            }
            Lookahead += 1.f; // can also make it double (ie. nonlinear increase)
            RemainingIters--;
        }
        ensure(Walkable == true || RemainingIters == 0);
        if (RemainingIters == 0)
        {
            /// TODO: fix linker error
            // Walker->PutActorToSleep(UCarlaStatics::GetCurrentEpisode(World)); // destroys this actor
        }

        struct FWalkerControl WalkerControl;
        { // create walker control following policy (stay on sidewalk)

            { // set forward speed
                WalkerControl.Speed = WS.Speed;
            }

            { // need to jump?
                if (Walkable)
                {
                    FHitResult RightBelow = DownGroundTrace(World, Location, PersonHeightMax, {WalkerActor});
                    AActor *RightBelowActor = RightBelow.Actor.Get();
                    if (RightBelowActor != nullptr && RightBelowActor->GetName().ToLower().Contains("curb"))
                    {
                        const float JumpThresh = 5.f; // cm threshold to jump
                        // if the walkable location is higher than the current location by a threshold
                        if (NewWalkableLocation.Z > RightBelow.Location.Z + JumpThresh) // perhaps on a curb/hill
                        {
                            WalkerControl.Jump = true;
                        }
                    }
                }
            }

            { // set forward direction (turn to Walkable point)
                // Direction stores the direction (not rotation!) of the vector which speed is applied to
                WalkerControl.Direction = Rotation.Vector(); // unit vector
                WalkerControl.Direction.Normalize();         // ensure unit length so speed is unchanged
            }
        }
        Walker->SetActorState(carla::rpc::ActorState::Active); // NOT dormant
        auto Response = Walker->ApplyControlToWalker(WalkerControl);
    }
}
