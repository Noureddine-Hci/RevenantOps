// Copyright RevenantOps. All Rights Reserved.

#include "ZombieSlow.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"

AZombieSlow::AZombieSlow() {
  // Slow walk speed - horde zombie
  GetCharacterMovement()->MaxWalkSpeed = 200.f;

  // Low melee damage, slow attack rate
  MeleeDamage = 10.f;
  MeleeAttackCooldown = 2.0f;

  // Display name
  EnemyName = FText::FromString(TEXT("Zombie Lent"));

  // Moderate health - survives a few shots
  if (HealthComp) {
    HealthComp->SetMaxHealth(80.f);
  }
}
