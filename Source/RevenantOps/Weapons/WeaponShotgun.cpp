// Copyright RevenantOps. All Rights Reserved.

#include "WeaponShotgun.h"

AWeaponShotgun::AWeaponShotgun() {
  // Identity
  WeaponName = FText::FromString(TEXT("Shotgun"));
  WeaponCategory = EWeaponCategory::Shotgun;

  // Firing
  FireMode = EWeaponFireMode::SemiAuto;
  FireRate = 80.f;
  PelletsPerShot = 8;
  MaxRange = 3000.f;

  // Damage
  BaseDamage = 15.f;
  HeadshotMultiplier = 1.5f;
  DamageFalloffStart = 500.f;
  DamageFalloffEnd = 1500.f;
  MinDamageMultiplier = 0.2f;

  // Ammo
  MagazineSize = 6;
  CurrentAmmo = 6;
  MaxReserveAmmo = 24;
  CurrentReserveAmmo = 24;
  ReloadTime = 3.0f;

  // Spread
  BaseSpread = 5.0f;
  MaxSpread = 8.f;
  SpreadPerShot = 0.5f;

  // Recoil
  VerticalRecoil = 2.5f;
  HorizontalRecoilRange = 1.0f;
  ADSRecoilMultiplier = 0.7f;

  // ADS
  ADSFOV = 65.f;
  ADSMoveSpeedMultiplier = 0.5f;
}
