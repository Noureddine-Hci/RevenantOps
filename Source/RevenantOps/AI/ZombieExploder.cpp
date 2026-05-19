// Copyright RevenantOps. All Rights Reserved.

#include "ZombieExploder.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// Stats globales viennent de DT_EnemyStats.
// ExplosionDamage/ExplosionRadius restent paramètres BP de cette classe.
AZombieExploder::AZombieExploder()
{
    EnemyName = FText::FromString(TEXT("Zombie Explosif"));
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

  // Play explosion sound
  if (ExplosionSound) {
    UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound,
                                           GetActorLocation());
  }

  // Spawn explosion VFX
  if (ExplosionVFX) {
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        this, ExplosionVFX, GetActorLocation(), FRotator::ZeroRotator,
        FVector(1.f), true);
  }

  // Notify Blueprint for VFX (particles, sound, camera shake)
  BP_OnExplode();

  // Kill self via ApplyDamage to preserve the death delegate chain
  // (HealthComponent->HandleTakeAnyDamage -> OnDeath -> EnemyBase::HandleDeath -> OnEnemyDied.Broadcast)
  // This ensures the wave spawner correctly tracks the death
  UGameplayStatics::ApplyDamage(this, 9999.f, GetController(), this, nullptr);
}
