// Copyright RevenantOps. All Rights Reserved.

#include "Audio/SoundManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void USoundManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SoundMap.Empty();
}

void USoundManagerSubsystem::RegisterSound(FName Key, USoundBase* Sound)
{
    if (Key.IsNone() || !Sound) return;
    SoundMap.Add(Key, Sound);
}

USoundBase* USoundManagerSubsystem::GetSound(FName Key) const
{
    USoundBase* const* Found = SoundMap.Find(Key);
    return Found ? *Found : nullptr;
}

bool USoundManagerSubsystem::Play(const UObject* WorldContext, FName Key,
                                  FVector Location, float VolumeMult, float PitchMult)
{
    USoundBase* Sound = GetSound(Key);
    if (!Sound)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[SoundMgr] Key '%s' not registered"), *Key.ToString());
        return false;
    }
    UGameplayStatics::PlaySoundAtLocation(WorldContext, Sound, Location, VolumeMult, PitchMult);
    return true;
}

bool USoundManagerSubsystem::Play2D(const UObject* WorldContext, FName Key,
                                    float VolumeMult, float PitchMult)
{
    USoundBase* Sound = GetSound(Key);
    if (!Sound)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[SoundMgr] Key '%s' not registered"), *Key.ToString());
        return false;
    }
    UGameplayStatics::PlaySound2D(WorldContext, Sound, VolumeMult, PitchMult);
    return true;
}
