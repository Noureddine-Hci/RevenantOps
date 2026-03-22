// Copyright RevenantOps. All Rights Reserved.

#include "WeaponSMG.h"

AWeaponSMG::AWeaponSMG() {
  // Identity
  WeaponName = FText::FromString(TEXT("SMG"));
  WeaponCategory = EWeaponCategory::SMG;

  // Firing
  FireMode = EWeaponFireMode::FullAuto;
  FireRate = 1000.f;
  MaxRange = 6000.f;

  // Damage
  BaseDamage = 12.f;
  DamageFalloffStart = 1000.f;
  DamageFalloffEnd = 3000.f;

  // Ammo
  MagazineSize = 40;
  CurrentAmmo = 40;
  MaxReserveAmmo = 160;
  CurrentReserveAmmo = 160;
  ReloadTime = 1.8f;

  // Recoil
  VerticalRecoil = 0.5f;
  HorizontalRecoilRange = 0.5f;

  // Spread
  BaseSpread = 3.0f;
  MaxSpread = 8.f;
  SpreadPerShot = 0.3f;

  // ADS
  ADSFOV = 65.f;
  ADSMoveSpeedMultiplier = 0.7f;
}
