// Copyright RevenantOps. All Rights Reserved.

#include "ZombieSpitter.h"
#include "ZombieProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

AZombieSpitter::AZombieSpitter() {
  // Slower than melee zombies — ranged unit
  GetCharacterMovement()->MaxWalkSpeed = 250.f;

  // Attack range: fires when player is within 1200cm
  MeleeRange = 1200.f;

  // Fires every 2.5 seconds
  MeleeAttackCooldown = 2.5f;

  // Damage comes from projectile, not melee hit
  MeleeDamage = 0.f;

  // Tries to stay at 1000cm from the player
  IdealEngagementRange = 1000.f;
  MaxEngagementRange = 1500.f;

  // Identity
  EnemyName = FText::FromString(TEXT("Zombie Cracheur"));

  // 100 HP
  if (HealthComp) {
    HealthComp->SetMaxHealth(100.f);
  }

  // Sees further than melee zombies
  SightRange = 2500.f;
}

void AZombieSpitter::Tick(float DeltaTime) {
  // Super::Tick runs perception + UpdateZombieCombat from ZombieBase
  Super::Tick(DeltaTime);

  // Distance-keeping: retreat when player gets too close
  if (AlertState == EEnemyAlertState::Alert && TargetPlayer) {
    const float Distance =
        FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    // Player is within 60% of ideal range — too close, retreat
    if (Distance < IdealEngagementRange * 0.6f) {
      FVector AwayFromPlayer =
          (GetActorLocation() - TargetPlayer->GetActorLocation())
              .GetSafeNormal();
      FVector RetreatTarget = GetActorLocation() + AwayFromPlayer * 400.f;

      AAIController* AIC = Cast<AAIController>(GetController());
      if (AIC) {
        AIC->MoveToLocation(RetreatTarget, 50.f);
      }
    }
  }
}

void AZombieSpitter::PerformMeleeAttack() {
  if (!TargetPlayer || !ProjectileClass) {
    return;
  }

  // Determine spawn location: try socket, fallback to offset from actor
  FVector SpawnLoc;
  if (GetMesh() && GetMesh()->DoesSocketExist(ProjectileSpawnSocket)) {
    SpawnLoc = GetMesh()->GetSocketLocation(ProjectileSpawnSocket);
  } else {
    SpawnLoc = GetActorLocation() + GetActorForwardVector() * 50.f +
               FVector(0.f, 0.f, 60.f);
  }

  // Calculate direction to player
  FVector Direction =
      (TargetPlayer->GetActorLocation() - SpawnLoc).GetSafeNormal();
  FRotator SpawnRot = Direction.Rotation();

  // Spawn the projectile
  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = this;
  SpawnParams.Instigator = this;

  AZombieProjectile* Proj = GetWorld()->SpawnActor<AZombieProjectile>(
      ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);

  if (Proj) {
    Proj->InitProjectile(ProjectileDamage, this);
  }

  // Notify Blueprint (inherited from ZombieBase — "attack performed" hook)
  BP_OnMeleeAttack();
}
