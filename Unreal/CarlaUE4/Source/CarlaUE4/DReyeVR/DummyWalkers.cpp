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
}

void DummyWalkers::FindWalkers(UWorld *World)
{
    auto Episode = UCarlaStatics::GetCurrentEpisode(World);
    ensure(Episode != nullptr);
    if (Episode == nullptr)
        return;

    const FName DummyWalkerTag{"DummyWalker"};
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
            if (Walkers.find(WalkerActor) == Walkers.end() && WalkerActor->WasRecentlyRendered())
            {
                Walkers[WalkerActor] = NewWalker(Actor);
            }
        }
    }
    const size_t NewSize = Walkers.size();
    if (NewSize > 0)
        UE_LOG(LogTemp, Log, TEXT("Updated Walker list to with %zu walkers"), NewSize);
}

void DummyWalkers::Tick(UWorld *World, const float DeltaSeconds)
{
    // loop over all relevent (rendered) walkers and give them some simple
    // policy to walk around the sidewalks

    if (TimeSinceLastActorRefresh == 0.f)
    {
        FindWalkers(World);
    }
    if (TimeSinceLastActorRefresh < RefreshActorSearchTick)
    {
        // keep incrementing the time since last refresh as long as the threshold has not been met
        TimeSinceLastActorRefresh += DeltaSeconds;
    }
    else
    {
        // reset the time since last actor was refreshed if beyond the tick threshold
        TimeSinceLastActorRefresh = 0.f;
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
            const FName DummyWalkerTag{"DummyWalker"};
            bool SkipWalker = !WalkerActor->WasRecentlyRendered(0.0f) || // optimization
                              !WalkerActor->ActorHasTag(DummyWalkerTag); // not part of this policy
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
        while (Walkable == false && RemainingIters > 0)
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
                if (!Walkable)
                { // begin rotation step to see if the next AngularStep is walkable
                    // rotate CW (arbitrary choice)
                    Rotation.Yaw += WS.AngularStep;
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

    // clear all the walkers to refresh on the next tick
    Walkers.clear();
}
