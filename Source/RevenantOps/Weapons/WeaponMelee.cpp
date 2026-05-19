// Copyright RevenantOps. All Rights Reserved.

#include "WeaponMelee.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Pawn.h"

// Stats globales (dégâts, fire rate, etc.) viennent de DT_WeaponStats.
// MeleeRange/MeleeRadius restent spécifiques à cette classe (paramètres BP).
AWeaponMelee::AWeaponMelee()
{
    WeaponName     = FText::FromString(TEXT("Machette"));
    WeaponCategory = EWeaponCategory::Melee;
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
