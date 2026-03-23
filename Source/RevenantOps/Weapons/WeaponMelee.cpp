// Copyright RevenantOps. All Rights Reserved.

#include "WeaponMelee.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Pawn.h"

AWeaponMelee::AWeaponMelee() {
  // Identity
  WeaponName = FText::FromString(TEXT("Machette"));
  WeaponCategory = EWeaponCategory::Melee;

  // Firing
  FireMode = EWeaponFireMode::SemiAuto;
  FireRate = 120.f; // 2 swings per second
  PelletsPerShot = 1;
  MaxRange = 100.f;

  // Damage — high risk/reward
  BaseDamage = 35.f;
  HeadshotMultiplier = 1.5f;
  DamageFalloffStart = 100.f;
  DamageFalloffEnd = 100.f;
  MinDamageMultiplier = 1.0f; // No falloff for melee

  // Ammo — infinite, never runs out
  MagazineSize = 999;
  CurrentAmmo = 999;
  MaxReserveAmmo = 0;
  CurrentReserveAmmo = 0;
  ReloadTime = 0.f;

  // Recoil — none for melee
  VerticalRecoil = 0.f;
  HorizontalRecoilRange = 0.f;

  // Spread — none for melee
  BaseSpread = 0.f;
  MaxSpread = 0.f;
  SpreadPerShot = 0.f;

  // ADS — no zoom, full speed
  ADSFOV = 90.f;
  ADSMoveSpeedMultiplier = 1.0f;

  // Melee-specific
  MeleeRange = 100.f;
  MeleeRadius = 50.f;
}

void AWeaponMelee::FireShot() {
  // Do NOT call Super::FireShot() — completely replace hitscan logic

  if (!OwnerPawn) {
    return;
  }

  // Keep ammo at 999 to prevent auto-reload edge cases
  CurrentAmmo = 999;

  LastFireTime = GetWorld()->GetTimeSeconds();
  SetWeaponState(EWeaponState::Firing);

  // Sphere trace from character center forward
  const FVector TraceStart = OwnerPawn->GetActorLocation();
  const FVector TraceEnd =
      TraceStart + OwnerPawn->GetActorForwardVector() * MeleeRange;

  FCollisionShape SphereShape = FCollisionShape::MakeSphere(MeleeRadius);

  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(this);
  QueryParams.AddIgnoredActor(OwnerPawn);
  QueryParams.bReturnPhysicalMaterial = true;

  FHitResult HitResult;
  const bool bHit = GetWorld()->SweepSingleByChannel(
      HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility,
      SphereShape, QueryParams);

  if (bHit && HitResult.GetActor()) {
    // Melee: full damage, no falloff
    const float Damage = BaseDamage;

    FPointDamageEvent DamageEvent(
        Damage, HitResult, OwnerPawn->GetActorForwardVector(), nullptr);
    HitResult.GetActor()->TakeDamage(Damage, DamageEvent, OwnerController,
                                      this);

    BP_OnHit(HitResult, Damage);
  }

  // Notify Blueprint for VFX/SFX
  BP_OnFire(OwnerPawn->GetActorLocation(), OwnerPawn->GetActorRotation());

  // SemiAuto behavior — one swing per input
  bWantsToFire = false;

  SetWeaponState(EWeaponState::Idle);

  // Broadcast ammo change to keep HUD consistent
  OnAmmoChanged.Broadcast(CurrentAmmo, MagazineSize);
}
