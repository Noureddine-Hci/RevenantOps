// Copyright RevenantOps. All Rights Reserved.

#include "ZombieBase.h"
#include "EnemyTableRow.h"
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

  // Apply DataTable stats after Super (which binds HealthComp events) per D-10
  ApplyEnemyDataRow();
}

void AZombieBase::ApplyEnemyDataRow()
{
    if (EnemyDataRow.IsNull())
    {
        // No DT assigned — keep constructor defaults (per D-13)
        return;
    }

    static const FString ContextString(TEXT("AZombieBase::ApplyEnemyDataRow"));
    const FEnemyTableRow* Row = EnemyDataRow.GetRow<FEnemyTableRow>(ContextString);

    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("ZombieBase [%s]: DataTable row '%s' not found. Using constructor defaults."),
            *GetName(), *EnemyDataRow.RowName.ToString());
        return;
    }

    // Map DT fields to zombie properties (per D-12)
    if (HealthComp)
    {
        HealthComp->SetMaxHealth(Row->MaxHP);
        HealthComp->ResetHealth();
    }
    MeleeDamage = Row->MeleeDamage;
    GetCharacterMovement()->MaxWalkSpeed = Row->MovementSpeed;
}

void AZombieBase::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Super returns early if dead, but that only exits EnemyBase::Tick.
  // Guard here so the corpse doesn't keep meleeing the player.
  if (HealthComp && HealthComp->IsDead()) {
    return;
  }

  UpdateZombieCombat(DeltaTime);
}

void AZombieBase::UpdateZombieCombat(float DeltaTime) {
  // Only fight when alert and have a target
  if (AlertState != EEnemyAlertState::Alert || !TargetPlayer) {
    return;
  }

  // Stun : immobilisé, aucune action
  if (bIsStunned) return;

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

  // Play melee attack sound
  if (MeleeAttackSound) {
    UGameplayStatics::PlaySoundAtLocation(this, MeleeAttackSound,
                                           GetActorLocation());
  }

  // Animation d'attaque via slot — retourne automatiquement à la locomotion
  if (AttackAnim)
  {
    if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
    {
      AI->PlaySlotAnimationAsDynamicMontage(AttackAnim, FName("DefaultSlot"), 0.1f, 0.1f, 1.f);
    }
  }

  // Apply melee damage après le délai de windup
  // (donne une fenêtre au joueur pour réagir / esquiver / contre-attaquer)
  if (AttackWindupTime > 0.f)
  {
    APawn* CapturedTarget = TargetPlayer;
    GetWorldTimerManager().SetTimer(AttackWindupTimer,
      [this, CapturedTarget]()
      {
        if (!bIsDead && IsValid(CapturedTarget))
        {
          UGameplayStatics::ApplyDamage(CapturedTarget, MeleeDamage,
                                        GetController(), this, nullptr);
        }
      },
      AttackWindupTime, false);
  }
  else
  {
    UGameplayStatics::ApplyDamage(TargetPlayer, MeleeDamage, GetController(),
                                  this, nullptr);
  }

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
