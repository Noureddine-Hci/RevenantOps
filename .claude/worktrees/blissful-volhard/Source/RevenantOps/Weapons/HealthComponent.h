// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

/**
 *  Delegate fired when health changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnHealthChanged, UHealthComponent *, HealthComp, float, Health,
    float, HealthDelta, const AController *, InstigatedBy);

/**
 *  Delegate fired when health reaches zero
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnDeath, UHealthComponent *, HealthComp, const AController *,
    InstigatedBy, AActor *, DamageCauser);

/**
 *  Reusable Health Component.
 *  Attach to any Actor (player, enemy, destructible) to give it health.
 *  Listens to TakeDamage on the owning actor automatically.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UHealthComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UHealthComponent();

protected:
  virtual void BeginPlay() override;

  // ========== HEALTH ==========

  /** Maximum health */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health",
            meta = (ClampMin = 1, ClampMax = 10000))
  float MaxHealth = 100.f;

  /** Current health */
  UPROPERTY(BlueprintReadOnly, Category = "Health")
  float CurrentHealth = 100.f;

  /** Is the owner currently dead */
  UPROPERTY(BlueprintReadOnly, Category = "Health")
  bool bIsDead = false;

  // ========== SHIELD ==========

  /** Maximum shield (absorbs damage before health) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Shield",
            meta = (ClampMin = 0, ClampMax = 5000))
  float MaxShield = 0.f;

  /** Current shield */
  UPROPERTY(BlueprintReadOnly, Category = "Health|Shield")
  float CurrentShield = 0.f;

  /** Shield regeneration rate per second */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Shield",
            meta = (ClampMin = 0, ClampMax = 200))
  float ShieldRegenRate = 10.f;

  /** Delay before shield starts regenerating after taking damage */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Shield",
            meta = (ClampMin = 0, ClampMax = 10))
  float ShieldRegenDelay = 3.f;

  /** Time of last damage received */
  float LastDamageTime = -100.f;

  // ========== DAMAGE MODIFIERS ==========

  /** Global damage multiplier (0.5 = take half damage) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Modifiers",
            meta = (ClampMin = 0.0, ClampMax = 10.0))
  float DamageMultiplier = 1.0f;

  /** If true, this actor is currently invulnerable */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Modifiers")
  bool bIsInvulnerable = false;

public:
  // ========== EVENTS ==========

  /** Broadcast when health changes */
  UPROPERTY(BlueprintAssignable, Category = "Health|Events")
  FOnHealthChanged OnHealthChanged;

  /** Broadcast when health reaches zero */
  UPROPERTY(BlueprintAssignable, Category = "Health|Events")
  FOnDeath OnDeath;

  // ========== PUBLIC API ==========

  /** Returns current health as a 0-1 percentage */
  UFUNCTION(BlueprintCallable, Category = "Health")
  float GetHealthPercent() const;

  /** Returns current shield as a 0-1 percentage */
  UFUNCTION(BlueprintCallable, Category = "Health")
  float GetShieldPercent() const;

  /** Returns true if the owner is dead */
  UFUNCTION(BlueprintCallable, Category = "Health")
  bool IsDead() const { return bIsDead; }

  /** Heals the owner by the specified amount */
  UFUNCTION(BlueprintCallable, Category = "Health")
  void Heal(float HealAmount);

  /** Restores health and shield to maximum */
  UFUNCTION(BlueprintCallable, Category = "Health")
  void ResetHealth();

  /** Getters */
  UFUNCTION(BlueprintCallable, Category = "Health")
  float GetCurrentHealth() const { return CurrentHealth; }

  UFUNCTION(BlueprintCallable, Category = "Health")
  float GetMaxHealth() const { return MaxHealth; }

  UFUNCTION(BlueprintCallable, Category = "Health")
  float GetCurrentShield() const { return CurrentShield; }

protected:
  /** Handles incoming damage from the owning actor's TakeDamage */
  UFUNCTION()
  void HandleTakeAnyDamage(AActor *DamagedActor, float Damage,
                           const class UDamageType *DamageType,
                           class AController *InstigatedBy,
                           AActor *DamageCauser);

  virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                             FActorComponentTickFunction *ThisTickFunction) override;
};
