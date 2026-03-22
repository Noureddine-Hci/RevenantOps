// Copyright RevenantOps. All Rights Reserved.

#include "ZombieRunner.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"

AZombieRunner::AZombieRunner() {
  // Fast sprint speed - charges at the player
  GetCharacterMovement()->MaxWalkSpeed = 600.f;

  // Higher melee damage, faster attack rate
  MeleeDamage = 15.f;
  MeleeAttackCooldown = 1.0f;

  // Slightly shorter melee range - hit-and-run style
  MeleeRange = 120.f;

  // Display name
  EnemyName = FText::FromString(TEXT("Zombie Rapide"));

  // Low health - glass cannon
  if (HealthComp) {
    HealthComp->SetMaxHealth(50.f);
  }
}
