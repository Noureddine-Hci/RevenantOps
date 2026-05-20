// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MenuTypes.h"
#include "WeaponBase.h"
#include "RevenantOpsGameInstance.generated.h"

class USettingsSaveGame;

/**
 * Game Instance — survives level changes.
 * Stores menu selections + persistent settings (loaded at boot).
 *
 * Setup: Project Settings → Maps & Modes → Game Instance Class = BP_RevenantOpsGameInstance
 */
UCLASS(Blueprintable)
class REVENANTOPS_API URevenantOpsGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    /** Level chosen in the level select screen */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    FLevelInfo PendingLevel;

    /** Character chosen in the character select screen */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    FCharacterInfo PendingCharacter;

    /** True when a level + character have been chosen and we're about to load */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    bool bPendingMatchStart = false;

    /** Armes choisies dans le loadout (ordre = ordre d'équipement) */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    TArray<TSubclassOf<AWeaponBase>> PendingWeapons;

    // ── SETTINGS PERSISTANTS ──────────────────────────────────────────────

    /** Settings chargés depuis le slot "Settings" (jamais null après Init) */
    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    USettingsSaveGame* Settings = nullptr;

    /** Persiste les settings courants sur disque (slot "Settings") */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SaveSettings();

    /** Applique les settings courants au monde (volumes audio, qualité graphique) */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplySettings();

protected:
    static const FString SettingsSlotName;
};
