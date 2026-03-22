// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

/**
 *  Projectile fired by ZombieSpitter.
 *  Travels toward the player with a slight arc, deals damage on hit,
 *  ignores other enemies (no friendly fire), and self-destructs after lifetime.
 */
UCLASS(Blueprintable)
class AZombieProjectile : public AActor {
  GENERATED_BODY()

public:
  AZombieProjectile();

  /** Called by ZombieSpitter after spawn to set damage and instigator */
  void InitProjectile(float Damage, AActor* ProjectileOwner);

protected:
  virtual void BeginPlay() override;

  // ========== COMPONENTS ==========

  /** Collision sphere for hit detection */
  UPROPERTY(VisibleAnywhere, Category = "Components")
  USphereComponent* CollisionSphere;

  /** Projectile movement for physics-based travel */
  UPROPERTY(VisibleAnywhere, Category = "Components")
  UProjectileMovementComponent* ProjectileMovement;

  // ========== PROPERTIES ==========

  /** Travel speed of the projectile */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
  float ProjectileSpeed = 1500.f;

  /** Auto-destroy after this many seconds */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
  float ProjectileLifetime = 5.f;

  /** Damage dealt on hit (set via InitProjectile) */
  float ProjectileDamage = 20.f;

  /** Reference to the actor that spawned this projectile */
  UPROPERTY()
  AActor* OwnerActor = nullptr;

  // ========== METHODS ==========

  /** Called when the projectile hits something */
  UFUNCTION()
  void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
             UPrimitiveComponent* OtherComp, FVector NormalImpulse,
             const FHitResult& Hit);

  /** Blueprint hook for VFX/SFX on hit (Phase 8) */
  UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events")
  void BP_OnProjectileHit(const FHitResult& Hit);
};
