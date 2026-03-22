// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
#include "ZombieSpitter.generated.h"

class AZombieProjectile;

/**
 *  Ranged zombie that fires projectiles from distance.
 *  Overrides melee attack to spawn projectiles and maintains distance
 *  from the player instead of charging. Retreats when player gets too close.
 */
UCLASS(Blueprintable)
class AZombieSpitter : public AZombieBase {
  GENERATED_BODY()

public:
  AZombieSpitter();

protected:
  // ========== RANGED ATTACK ==========

  /** Projectile class to spawn (must be set in Blueprint) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Ranged")
  TSubclassOf<AZombieProjectile> ProjectileClass;

  /** Damage dealt by each projectile */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Ranged")
  float ProjectileDamage = 20.f;

  /** Socket on the skeleton mesh to spawn projectile from (mouth area) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Ranged")
  FName ProjectileSpawnSocket = FName("head");

  // ========== OVERRIDES ==========

  /** Spawns a projectile instead of performing melee damage */
  virtual void PerformMeleeAttack() override;

  /** Adds distance-keeping (retreat) behavior */
  virtual void Tick(float DeltaTime) override;
};
