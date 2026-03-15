// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RevenantOpsSaveGame.generated.h"

/**
 *  Save data for player state
 */
USTRUCT(BlueprintType)
struct FPlayerSaveData {
  GENERATED_BODY()

  /** Player location in the world */
  UPROPERTY(BlueprintReadWrite)
  FTransform PlayerTransform;

  /** Current health */
  UPROPERTY(BlueprintReadWrite)
  float CurrentHealth = 100.f;

  /** Current shield */
  UPROPERTY(BlueprintReadWrite)
  float CurrentShield = 0.f;

  /** Current stamina */
  UPROPERTY(BlueprintReadWrite)
  float CurrentStamina = 100.f;

  /** Index of equipped weapon */
  UPROPERTY(BlueprintReadWrite)
  int32 EquippedWeaponIndex = 0;

  /** Ammo per weapon (index matches weapon inventory) */
  UPROPERTY(BlueprintReadWrite)
  TArray<int32> WeaponAmmo;

  /** Reserve ammo per weapon */
  UPROPERTY(BlueprintReadWrite)
  TArray<int32> WeaponReserveAmmo;
};

/**
 *  Save data for the game world
 */
USTRUCT(BlueprintType)
struct FWorldSaveData {
  GENERATED_BODY()

  /** Name of the current level */
  UPROPERTY(BlueprintReadWrite)
  FName CurrentLevelName;

  /** Checkpoint ID (the last checkpoint reached) */
  UPROPERTY(BlueprintReadWrite)
  FName LastCheckpointID;

  /** IDs of destroyed objects (so they don't respawn on load) */
  UPROPERTY(BlueprintReadWrite)
  TArray<FName> DestroyedObjectIDs;

  /** IDs of collected pickups */
  UPROPERTY(BlueprintReadWrite)
  TArray<FName> CollectedPickupIDs;

  /** IDs of completed encounters */
  UPROPERTY(BlueprintReadWrite)
  TArray<FName> CompletedEncounterIDs;
};

/**
 *  Main SaveGame class for RevenantOps.
 *  Stores player state and world progress.
 */
UCLASS()
class URevenantOpsSaveGame : public USaveGame {
  GENERATED_BODY()

public:
  URevenantOpsSaveGame();

  /** Player data */
  UPROPERTY(BlueprintReadWrite, Category = "Save")
  FPlayerSaveData PlayerData;

  /** World data */
  UPROPERTY(BlueprintReadWrite, Category = "Save")
  FWorldSaveData WorldData;

  /** Total play time in seconds */
  UPROPERTY(BlueprintReadWrite, Category = "Save")
  float TotalPlayTime = 0.f;

  /** Save timestamp */
  UPROPERTY(BlueprintReadWrite, Category = "Save")
  FDateTime SaveTimestamp;

  /** Save slot display name */
  UPROPERTY(BlueprintReadWrite, Category = "Save")
  FString SaveDisplayName;
};
