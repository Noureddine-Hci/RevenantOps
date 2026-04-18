// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "InventoryItem.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemType : uint8 {
  Empty     UMETA(DisplayName = "Vide"),
  Weapon    UMETA(DisplayName = "Arme"),
  Health    UMETA(DisplayName = "Soin"),
  Ammo      UMETA(DisplayName = "Munitions"),
  TimeBonus UMETA(DisplayName = "Bonus Temps"),
};

USTRUCT(BlueprintType)
struct REVENANTOPS_API FInventoryItem {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  EInventoryItemType Type = EInventoryItemType::Empty;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  FText DisplayName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  FText Description;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  int32 Quantity = 0;

  /** For weapon slots */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  TSubclassOf<class AWeaponBase> WeaponClass;

  /** Heal amount (for Health type) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  float HealAmount = 0.f;

  /** Time bonus in seconds (for TimeBonus type) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  float TimeBonusSeconds = 0.f;

  /** Icone affichee dans le slot (optionnel — fallback sur couleur de type) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  UTexture2D* ItemIcon = nullptr;

  bool IsEmpty() const { return Type == EInventoryItemType::Empty; }
};
