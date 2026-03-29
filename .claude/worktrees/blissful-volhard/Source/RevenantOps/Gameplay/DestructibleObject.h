// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleObject.generated.h"

class UHealthComponent;
class UStaticMeshComponent;

/**
 *  Delegate fired when destroyed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectDestroyed,
                                             ADestructibleObject *, Object);

/**
 *  Destructible object (barrels, crates, walls, etc.)
 *  Uses HealthComponent for damage tracking.
 *  Can drop items on destruction.
 */
UCLASS(Blueprintable)
class ADestructibleObject : public AActor {
  GENERATED_BODY()

public:
  ADestructibleObject();

protected:
  virtual void BeginPlay() override;

  // ========== COMPONENTS ==========

  /** Visual mesh */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *ObjectMesh;

  /** Health component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UHealthComponent *HealthComp;

  // ========== CONFIG ==========

  /** If true, deals explosion damage when destroyed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible")
  bool bExplodesOnDestruction = false;

  /** Explosion damage */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible",
            meta = (ClampMin = 0, ClampMax = 500, EditCondition = "bExplodesOnDestruction"))
  float ExplosionDamage = 50.f;

  /** Explosion radius */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible",
            meta = (ClampMin = 0, ClampMax = 2000, EditCondition = "bExplodesOnDestruction"))
  float ExplosionRadius = 300.f;

  /** Loot to drop on destruction (Actor classes) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot")
  TArray<TSubclassOf<AActor>> LootDrops;

  /** Chance to drop each loot item (0-1) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot",
            meta = (ClampMin = 0.0, ClampMax = 1.0))
  float LootDropChance = 0.5f;

public:
  UPROPERTY(BlueprintAssignable, Category = "Destructible|Events")
  FOnObjectDestroyed OnObjectDestroyed;

protected:
  /** Called when health reaches zero */
  UFUNCTION()
  void HandleDeath(UHealthComponent *HealthComponent,
                   const AController *InstigatedBy, AActor *DamageCauser);

  /** Spawns loot drops */
  void SpawnLoot();

  /** Deals explosion damage to nearby actors */
  void ApplyExplosionDamage(AController *InstigatedBy);

  /** BP hook for destruction VFX */
  UFUNCTION(BlueprintImplementableEvent, Category = "Destructible",
            meta = (DisplayName = "On Destroyed"))
  void BP_OnDestroyed();
};
