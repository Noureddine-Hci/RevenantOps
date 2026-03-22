// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
#include "ZombieExploder.generated.h"

/**
 *  Exploder zombie - fast runner that detonates on proximity.
 *  Runs at speed 350, low HP (60), explodes dealing 50 AoE damage
 *  within 300cm radius when reaching the player, then self-destructs.
 */
UCLASS(Blueprintable)
class AZombieExploder : public AZombieBase {
  GENERATED_BODY()

public:
  AZombieExploder();

protected:
  // ========== EXPLOSION PROPERTIES ==========

  /** Radius of the explosion in cm */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Explosion",
            meta = (ClampMin = 100, ClampMax = 1000))
  float ExplosionRadius = 300.f;

  /** Damage dealt by the explosion */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Explosion",
            meta = (ClampMin = 10, ClampMax = 200))
  float ExplosionDamage = 50.f;

  /** Prevents double explosion */
  bool bHasExploded = false;

  // ========== OVERRIDES ==========

  /** Overrides melee attack to explode instead of swinging */
  virtual void PerformMeleeAttack() override;

  // ========== BLUEPRINT HOOKS ==========

  /** Called when the zombie explodes, for Blueprint VFX */
  UFUNCTION(BlueprintImplementableEvent, Category = "Zombie|Events")
  void BP_OnExplode();
};
