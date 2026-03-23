// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class AWeaponBase;

/**
 *  Weapon data for display in the loadout selection screen.
 */
USTRUCT(BlueprintType)
struct FWeaponLoadoutInfo {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FText WeaponName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AWeaponBase> WeaponClass;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Damage = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float FireRate = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 MagazineSize = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float ReloadTime = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *WeaponIcon = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadoutConfirmed,
                                              TSubclassOf<AWeaponBase>,
                                              PrimaryWeapon,
                                              TSubclassOf<AWeaponBase>,
                                              SecondaryWeapon);

/**
 *  Loadout selection screen.
 *  Player picks 2 firearms from available weapons. Melee is always equipped.
 *  Shows weapon stats (damage, fire rate, magazine, reload time).
 */
UCLASS(abstract, Blueprintable)
class ULoadoutWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void NativeConstruct() override;

  // ========== CONFIGURATION ==========

  /** Available weapons for selection (set in Blueprint or from GameMode) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
  TArray<FWeaponLoadoutInfo> AvailableWeapons;

  /** Melee weapon class (always equipped, not selectable) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
  TSubclassOf<AWeaponBase> MeleeWeaponClass;

  // ========== SELECTION STATE ==========

  /** Index of selected primary weapon (-1 = none) */
  UPROPERTY(BlueprintReadOnly, Category = "Loadout")
  int32 PrimaryWeaponIndex = -1;

  /** Index of selected secondary weapon (-1 = none) */
  UPROPERTY(BlueprintReadOnly, Category = "Loadout")
  int32 SecondaryWeaponIndex = -1;

  // ========== EVENTS ==========

  /** Fired when player confirms loadout */
  UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
  FOnLoadoutConfirmed OnLoadoutConfirmed;

  // ========== API ==========

  /** Select a weapon for the given slot (0 = primary, 1 = secondary) */
  UFUNCTION(BlueprintCallable, Category = "Loadout")
  void SelectWeapon(int32 WeaponIndex, int32 SlotIndex);

  /** Confirm loadout and broadcast event */
  UFUNCTION(BlueprintCallable, Category = "Loadout")
  void ConfirmLoadout();

  /** Can the player confirm (both slots filled, different weapons) */
  UFUNCTION(BlueprintCallable, Category = "Loadout")
  bool CanConfirm() const;

  /** Get info about a weapon by index */
  UFUNCTION(BlueprintCallable, Category = "Loadout")
  FWeaponLoadoutInfo GetWeaponInfo(int32 Index) const;

  /** Populate AvailableWeapons from an array of weapon classes (reads CDO stats) */
  UFUNCTION(BlueprintCallable, Category = "Loadout")
  void PopulateFromClasses(
      const TArray<TSubclassOf<AWeaponBase>> &WeaponClasses);

protected:
  /** BP hook: called when selection changes (for UI update) */
  UFUNCTION(BlueprintImplementableEvent, Category = "Loadout",
            meta = (DisplayName = "On Selection Changed"))
  void BP_OnSelectionChanged(int32 SlotIndex, int32 WeaponIndex);

  /** BP hook: called when loadout is confirmed */
  UFUNCTION(BlueprintImplementableEvent, Category = "Loadout",
            meta = (DisplayName = "On Loadout Confirmed"))
  void BP_OnLoadoutConfirmed();
};
