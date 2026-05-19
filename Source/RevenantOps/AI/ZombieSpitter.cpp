// Copyright RevenantOps. All Rights Reserved.

#include "ZombieSpitter.h"
#include "ZombieProjectile.h"
#include "AIController.h"

// Stats globales (HP, vitesse, mêlée) viennent de DT_EnemyStats.
// IdealEngagementRange/MaxEngagementRange/SightRange restent paramètres BP de cette classe.
AZombieSpitter::AZombieSpitter()
{
    EnemyName = FText::FromString(TEXT("Zombie Cracheur"));
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
