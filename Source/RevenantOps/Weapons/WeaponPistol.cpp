// Copyright RevenantOps. All Rights Reserved.

#include "WeaponPistol.h"

AWeaponPistol::AWeaponPistol() {
  // Identity
  WeaponName = FText::FromString(TEXT("Pistol"));
  WeaponCategory = EWeaponCategory::Pistol;

  // Firing
  FireMode = EWeaponFireMode::SemiAuto;
  FireRate = 400.f;
  MaxRange = 8000.f;

  // Damage
  BaseDamage = 25.f;
  DamageFalloffStart = 1500.f;
  DamageFalloffEnd = 4000.f;

  // Ammo
  MagazineSize = 12;
  CurrentAmmo = 12;
  MaxReserveAmmo = 60;
  CurrentReserveAmmo = 60;
  ReloadTime = 1.5f;

  // Recoil
  VerticalRecoil = 1.0f;
  HorizontalRecoilRange = 0.4f;

  // Spread
  BaseSpread = 1.5f;
  MaxSpread = 4.f;
  SpreadPerShot = 0.3f;

  // ADS
  ADSFOV = 65.f;
  ADSMoveSpeedMultiplier = 0.7f;
}
