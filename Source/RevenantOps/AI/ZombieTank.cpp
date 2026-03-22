// Copyright RevenantOps. All Rights Reserved.

#include "ZombieTank.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"

AZombieTank::AZombieTank() {
  // Very slow walk speed - lumbering tank
  GetCharacterMovement()->MaxWalkSpeed = 150.f;

  // Heavy melee damage, slow swings
  MeleeDamage = 30.f;
  MeleeRange = 180.f;
  MeleeAttackCooldown = 2.5f;

  // Display name
  EnemyName = FText::FromString(TEXT("Zombie Tank"));

  // Very high health - damage sponge
  if (HealthComp) {
    HealthComp->SetMaxHealth(300.f);
  }
}
