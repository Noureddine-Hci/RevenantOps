// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Pawn.h"
#include "MenuTypes.generated.h"

/** Describes a playable level shown in the level select screen */
USTRUCT(BlueprintType)
struct FLevelInfo
{
    GENERATED_BODY()

    /** Displayed name on the card (e.g. "The Compound") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Level")
    FText DisplayName;

    /** Map asset name passed to OpenLevel (e.g. "Lvl_ThirdPerson") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Level")
    FName MapName;

    /** Optional thumbnail shown on the card */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Level")
    UTexture2D* Thumbnail = nullptr;
};

/** Describes a selectable character shown in the character select screen */
USTRUCT(BlueprintType)
struct FCharacterInfo
{
    GENERATED_BODY()

    /** Displayed name on the card (e.g. "Mercenaire") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Character")
    FText DisplayName;

    /** Pawn class to spawn — assign BP_ThirdPersonCharacter or other variants */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Character")
    TSubclassOf<APawn> CharacterClass;

    /** Optional thumbnail shown on the card */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Character")
    UTexture2D* Thumbnail = nullptr;
};

/**
 * Describes a key binding that can be remapped in the Options menu.
 * Fill AvailableRebinds on BP_ThirdPersonPlayerController.
 *
 * Requirements for live rebinding:
 *   1. Project Settings → Enhanced Input → Enable User Settings = true
 *   2. Each Input Action asset → Player Mappable = true + set a MappingName
 */
USTRUCT(BlueprintType)
struct FKeyRebindEntry
{
    GENERATED_BODY()

    /** Human-readable label shown in the options row */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Options")
    FText DisplayName;

    /** Must match the PlayerMappableKeyOptions.Name on the Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Options")
    FName MappingName;

    /** Default key shown when no user override exists */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Options")
    FKey DefaultKey;
};
