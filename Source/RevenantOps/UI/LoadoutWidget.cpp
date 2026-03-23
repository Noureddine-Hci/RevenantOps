// Copyright RevenantOps. All Rights Reserved.

#include "LoadoutWidget.h"
#include "WeaponBase.h"

void ULoadoutWidget::NativeConstruct() {
  Super::NativeConstruct();

  PrimaryWeaponIndex = -1;
  SecondaryWeaponIndex = -1;
}

void ULoadoutWidget::SelectWeapon(int32 WeaponIndex, int32 SlotIndex) {
  if (WeaponIndex < 0 || WeaponIndex >= AvailableWeapons.Num()) {
    return;
  }

  if (SlotIndex == 0) {
    // Don't allow same weapon in both slots
    if (WeaponIndex == SecondaryWeaponIndex) {
      SecondaryWeaponIndex = -1;
    }
    PrimaryWeaponIndex = WeaponIndex;
  } else if (SlotIndex == 1) {
    if (WeaponIndex == PrimaryWeaponIndex) {
      PrimaryWeaponIndex = -1;
    }
    SecondaryWeaponIndex = WeaponIndex;
  }

  BP_OnSelectionChanged(SlotIndex, WeaponIndex);
}

void ULoadoutWidget::ConfirmLoadout() {
  if (!CanConfirm()) {
    return;
  }

  const TSubclassOf<AWeaponBase> Primary =
      AvailableWeapons[PrimaryWeaponIndex].WeaponClass;
  const TSubclassOf<AWeaponBase> Secondary =
      AvailableWeapons[SecondaryWeaponIndex].WeaponClass;

  OnLoadoutConfirmed.Broadcast(Primary, Secondary);
  BP_OnLoadoutConfirmed();
}

bool ULoadoutWidget::CanConfirm() const {
  return PrimaryWeaponIndex >= 0 && SecondaryWeaponIndex >= 0 &&
         PrimaryWeaponIndex != SecondaryWeaponIndex &&
         PrimaryWeaponIndex < AvailableWeapons.Num() &&
         SecondaryWeaponIndex < AvailableWeapons.Num();
}

FWeaponLoadoutInfo ULoadoutWidget::GetWeaponInfo(int32 Index) const {
  if (Index >= 0 && Index < AvailableWeapons.Num()) {
    return AvailableWeapons[Index];
  }
  return FWeaponLoadoutInfo();
}

void ULoadoutWidget::PopulateFromClasses(
    const TArray<TSubclassOf<AWeaponBase>> &WeaponClasses) {
  AvailableWeapons.Empty();

  for (const TSubclassOf<AWeaponBase> &WeapClass : WeaponClasses) {
    if (!WeapClass) {
      continue;
    }

    const AWeaponBase *CDO = WeapClass->GetDefaultObject<AWeaponBase>();
    if (!CDO) {
      continue;
    }

    FWeaponLoadoutInfo Info;
    Info.WeaponClass = WeapClass;
    Info.WeaponName = CDO->GetWeaponName();
    Info.Damage = CDO->GetBaseDamage();
    Info.FireRate = CDO->GetFireRate();
    Info.MagazineSize = CDO->GetMagazineSize();
    Info.ReloadTime = CDO->GetReloadTime();
    Info.WeaponIcon = nullptr; // Set in Blueprint

    AvailableWeapons.Add(Info);
  }
}
