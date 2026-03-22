// Copyright RevenantOps. All Rights Reserved.

#include "WeaponAssaultRifle.h"

AWeaponAssaultRifle::AWeaponAssaultRifle() {
  // Identity
  WeaponName = FText::FromString(TEXT("Assault Rifle"));
  WeaponCategory = EWeaponCategory::AssaultRifle;

  // Firing
  FireMode = EWeaponFireMode::FullAuto;
  FireRate = 700.f;
  MaxRange = 10000.f;

  // Damage
  BaseDamage = 18.f;
  DamageFalloffStart = 2000.f;
  DamageFalloffEnd = 5000.f;

  // Ammo
  MagazineSize = 30;
  CurrentAmmo = 30;
  MaxReserveAmmo = 120;
  CurrentReserveAmmo = 120;
  ReloadTime = 2.0f;

  // Recoil
  VerticalRecoil = 0.8f;
  HorizontalRecoilRange = 0.3f;

  // Spread
  BaseSpread = 2.0f;
  MaxSpread = 6.f;
  SpreadPerShot = 0.4f;

  // ADS
  ADSFOV = 60.f;
  ADSMoveSpeedMultiplier = 0.6f;
}
