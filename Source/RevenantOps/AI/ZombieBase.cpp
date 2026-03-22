// Copyright RevenantOps. All Rights Reserved.

#include "ZombieBase.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HealthComponent.h"

AZombieBase::AZombieBase() {
  // Zombies are always aggressive chargers
  BehaviorProfile = EEnemyBehavior::Aggressive;

  // Zombies don't use weapons - melee only
  WeaponClass = nullptr;

  // Perception: closer range but much wider FOV
  SightRange = 2000.f;
  SightHalfAngle = 120.f;

  // Engagement: melee range
  IdealEngagementRange = 100.f;
  MaxEngagementRange = 100.f;

  // Zombies react fast (instinct, not tactical)
  ReactionTime = 0.2f;
}

void AZombieBase::BeginPlay() {
  Super::BeginPlay();
}

void AZombieBase::Tick(float DeltaTime) {
  // Super::Tick runs UpdatePerception and UpdateCombat from EnemyBase.
  // UpdateCombat will try to fire, but EquippedWeapon is null so FireAtPlayer
  // returns early. This is harmless.
  Super::Tick(DeltaTime);

  // Run melee combat logic after perception has updated
  UpdateZombieCombat(DeltaTime);
}

void AZombieBase::UpdateZombieCombat(float DeltaTime) {
  // Only fight when alert and have a target
  if (AlertState != EEnemyAlertState::Alert || !TargetPlayer) {
    return;
  }

  // Decrement cooldown timer
  MeleeAttackTimer = FMath::Max(0.f, MeleeAttackTimer - DeltaTime);

  // Calculate distance to player
  const float DistanceToPlayer =
      FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

  if (DistanceToPlayer <= MeleeRange && MeleeAttackTimer <= 0.f) {
    // In melee range and cooldown is ready - attack!
    PerformMeleeAttack();
    MeleeAttackTimer = MeleeAttackCooldown;
    bIsAttacking = true;
  } else {
    // Not in range or on cooldown - charge at player
    MoveDirectlyToPlayer();
    bIsAttacking = false;
  }
}

void AZombieBase::PerformMeleeAttack() {
  if (!TargetPlayer) {
    return;
  }

  // Apply melee damage to the player
  UGameplayStatics::ApplyDamage(TargetPlayer, MeleeDamage, GetController(),
                                this, nullptr);

  // Notify Blueprint for effects (animation, sound, VFX)
  BP_OnMeleeAttack();
}

void AZombieBase::MoveDirectlyToPlayer() {
  if (!TargetPlayer) {
    return;
  }

  AAIController *AIC = Cast<AAIController>(GetController());
  if (AIC) {
    AIC->MoveToActor(TargetPlayer, MeleeRange * 0.5f);
  }
}
