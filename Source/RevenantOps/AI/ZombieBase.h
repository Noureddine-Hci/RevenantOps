// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "Engine/DataTable.h"
#include "ZombieBase.generated.h"

/**
 *  Abstract base class for all zombie types.
 *  Replaces gun-based combat with melee attacks.
 *  Zombies charge directly at the player and deal damage at close range.
 */
UCLASS(abstract, Blueprintable)
class AZombieBase : public AEnemyBase {
  GENERATED_BODY()

public:
  AZombieBase();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ========== DATATABLE ==========

  /** DataTable row handle — set in BP Defaults to {DT_EnemyStats, "RowName"} */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|Data")
  FDataTableRowHandle EnemyDataRow;

  // ========== MELEE COMBAT ==========

  /** Damage dealt per melee hit */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat",
            meta = (ClampMin = 1, ClampMax = 200))
  float MeleeDamage = 10.f;

  /** Range at which melee attack connects */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat",
            meta = (ClampMin = 50, ClampMax = 500))
  float MeleeRange = 150.f;

  /** Cooldown between melee attacks in seconds */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat",
            meta = (ClampMin = 0.1, ClampMax = 5.0))
  float MeleeAttackCooldown = 1.5f;

  /** Sound played on melee attack */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Audio")
  USoundBase *MeleeAttackSound = nullptr;

  /** Internal cooldown tracker */
  float MeleeAttackTimer = 0.f;

  /** True while performing a melee attack */
  bool bIsAttacking = false;

  // ========== ZOMBIE AI ==========

  /** Performs a melee attack on the target player. Virtual so subclasses can override (e.g. exploder). */
  virtual void PerformMeleeAttack();

  /** Reads balance stats from DataTable row. Called in BeginPlay after Super. */
  void ApplyEnemyDataRow();

  /** Melee combat update: attack if in range, otherwise charge at player */
  void UpdateZombieCombat(float DeltaTime);

  /** Simplified movement: charge directly at the player */
  void MoveDirectlyToPlayer();

  // ========== BLUEPRINT HOOKS ==========

  /** Called when a melee attack lands, for Blueprint effects (animation, sound, VFX) */
  UFUNCTION(BlueprintImplementableEvent, Category = "Zombie|Events")
  void BP_OnMeleeAttack();
};
