// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/MenuTypes.h"
#include "WeaponBase.h"
#include "RevenantOpsGameInstance.generated.h"

/**
 * Game Instance — survives level changes.
 * Stores the level and character selected in the menus so the new level
 * can read them back in ReceivedPlayer().
 *
 * Setup: Project Settings → Maps & Modes → Game Instance Class = BP_RevenantOpsGameInstance
 * (create a BP child of this class or assign directly if non-abstract)
 */
UCLASS(Blueprintable)
class REVENANTOPS_API URevenantOpsGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    /** Level chosen in the level select screen */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    FLevelInfo PendingLevel;

    /** Character chosen in the character select screen */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    FCharacterInfo PendingCharacter;

    /** True when a level + character have been chosen and we're about to load */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    bool bPendingMatchStart = false;

    /** Armes choisies dans le loadout */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    TSubclassOf<AWeaponBase> PendingPrimaryWeapon;

    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    TSubclassOf<AWeaponBase> PendingSecondaryWeapon;
};
