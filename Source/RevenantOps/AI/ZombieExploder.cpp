// Copyright RevenantOps. All Rights Reserved.

#include "ZombieExploder.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

AZombieExploder::AZombieExploder() {
  // Fast runner - rushes at player
  GetCharacterMovement()->MaxWalkSpeed = 350.f;

  // Detonation proximity (same as explosion radius)
  MeleeRange = 300.f;

  // Explodes immediately when in range
  MeleeAttackCooldown = 0.f;

  // No melee damage - damage comes from explosion
  MeleeDamage = 0.f;

  // Display name
  EnemyName = FText::FromString(TEXT("Zombie Explosif"));

  // Low health - fragile but fast
  if (HealthComp) {
    HealthComp->SetMaxHealth(60.f);
  }
}

void AZombieExploder::PerformMeleeAttack() {
  // Prevent double explosion
  if (bHasExploded) {
    return;
  }
  bHasExploded = true;

  // Apply radial damage to everything in explosion radius
  // Empty IgnoreActors array means everything in radius takes damage (including the player)
  // bDoFullDamage = true means full damage regardless of distance within radius
  UGameplayStatics::ApplyRadialDamage(
      this,                    // WorldContextObject
      ExplosionDamage,         // BaseDamage
      GetActorLocation(),      // Origin
      ExplosionRadius,         // DamageRadius
      nullptr,                 // DamageTypeClass
      TArray<AActor*>(),       // IgnoreActors (empty = damage everything)
      this,                    // DamageCauser
      GetController(),         // InstigatedByController
      true,                    // bDoFullDamage
      ECollisionChannel::ECC_Visibility  // DamagePreventionChannel
  );

  // Notify Blueprint for VFX (particles, sound, camera shake)
  BP_OnExplode();

  // Kill self via ApplyDamage to preserve the death delegate chain
  // (HealthComponent->HandleTakeAnyDamage -> OnDeath -> EnemyBase::HandleDeath -> OnEnemyDied.Broadcast)
  // This ensures the wave spawner correctly tracks the death
  UGameplayStatics::ApplyDamage(this, 9999.f, GetController(), this, nullptr);
}
