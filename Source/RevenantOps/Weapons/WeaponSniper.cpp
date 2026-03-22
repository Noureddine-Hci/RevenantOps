// Copyright RevenantOps. All Rights Reserved.

#include "WeaponSniper.h"

AWeaponSniper::AWeaponSniper() {
  // Identity
  WeaponName = FText::FromString(TEXT("Sniper"));
  WeaponCategory = EWeaponCategory::Sniper;

  // Firing
  FireMode = EWeaponFireMode::SemiAuto;
  FireRate = 40.f;
  PelletsPerShot = 1;
  MaxRange = 50000.f;

  // Damage
  BaseDamage = 120.f;
  HeadshotMultiplier = 3.0f;
  DamageFalloffStart = 5000.f;
  DamageFalloffEnd = 30000.f;
  MinDamageMultiplier = 0.7f;

  // Ammo
  MagazineSize = 5;
  CurrentAmmo = 5;
  MaxReserveAmmo = 20;
  CurrentReserveAmmo = 20;
  ReloadTime = 2.5f;

  // Spread
  BaseSpread = 0.2f;
  MaxSpread = 3.f;
  SpreadPerShot = 1.5f;
  ADSSpreadMultiplier = 0.1f;

  // Recoil
  VerticalRecoil = 3.0f;
  HorizontalRecoilRange = 0.5f;
  ADSRecoilMultiplier = 0.5f;

  // ADS
  ADSFOV = 30.f;
  ADSInterpSpeed = 8.f;
  ADSMoveSpeedMultiplier = 0.4f;
}
