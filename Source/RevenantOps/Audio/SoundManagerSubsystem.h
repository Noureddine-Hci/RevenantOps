// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SoundManagerSubsystem.generated.h"

class USoundBase;

/**
 *  GameInstance subsystem qui centralise les sons globaux du jeu.
 *  Évite d'assigner les mêmes USoundBase à chaque arme / ennemi / pickup.
 *
 *  Usage :
 *      USoundManagerSubsystem* SoundMgr = GetGameInstance()->GetSubsystem<USoundManagerSubsystem>();
 *      SoundMgr->Play(this, TEXT("Weapon.Pistol.Fire"), GetActorLocation());
 *
 *  Les sons sont chargés depuis une DataTable optionnelle (DT_SoundCues) ou
 *  enregistrés manuellement via RegisterSound() au boot.
 */
UCLASS()
class REVENANTOPS_API USoundManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Enregistre un son sous une clé textuelle (ex: "UI.Menu.Hover") */
    UFUNCTION(BlueprintCallable, Category = "Sound")
    void RegisterSound(FName Key, USoundBase* Sound);

    /** Joue un son 3D à la position donnée. Retourne false si la clé est inconnue. */
    UFUNCTION(BlueprintCallable, Category = "Sound", meta = (WorldContext = "WorldContext"))
    bool Play(const UObject* WorldContext, FName Key, FVector Location,
              float VolumeMult = 1.f, float PitchMult = 1.f);

    /** Joue un son 2D (UI, annonceur). Retourne false si la clé est inconnue. */
    UFUNCTION(BlueprintCallable, Category = "Sound", meta = (WorldContext = "WorldContext"))
    bool Play2D(const UObject* WorldContext, FName Key,
                float VolumeMult = 1.f, float PitchMult = 1.f);

    /** Retourne le USoundBase associé (peut être null si non enregistré) */
    UFUNCTION(BlueprintCallable, Category = "Sound")
    USoundBase* GetSound(FName Key) const;

private:
    UPROPERTY()
    TMap<FName, USoundBase*> SoundMap;
};
