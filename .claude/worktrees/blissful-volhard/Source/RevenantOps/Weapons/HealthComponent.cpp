// Copyright RevenantOps. All Rights Reserved.

#include "HealthComponent.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent() {
  PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComponent::BeginPlay() {
  Super::BeginPlay();

  // Initialize health and shield
  CurrentHealth = MaxHealth;
  CurrentShield = MaxShield;
  bIsDead = false;

  // Bind to owner's TakeDamage
  if (AActor *Owner = GetOwner()) {
    Owner->OnTakeAnyDamage.AddDynamic(
        this, &UHealthComponent::HandleTakeAnyDamage);
  }
}

void UHealthComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Shield regeneration
  if (MaxShield > 0.f && CurrentShield < MaxShield && !bIsDead) {
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastDamageTime >= ShieldRegenDelay) {
      CurrentShield =
          FMath::Min(CurrentShield + ShieldRegenRate * DeltaTime, MaxShield);
    }
  }
}

// =============================================================================
// DAMAGE HANDLING
// =============================================================================

void UHealthComponent::HandleTakeAnyDamage(AActor *DamagedActor, float Damage,
                                            const UDamageType *DamageType,
                                            AController *InstigatedBy,
                                            AActor *DamageCauser) {
  if (bIsDead || bIsInvulnerable || Damage <= 0.f) {
    return;
  }

  // Apply damage multiplier
  float ActualDamage = Damage * DamageMultiplier;

  // Record damage time for shield regen delay
  LastDamageTime = GetWorld()->GetTimeSeconds();

  // Shield absorbs damage first
  if (CurrentShield > 0.f) {
    const float ShieldDamage = FMath::Min(ActualDamage, CurrentShield);
    CurrentShield -= ShieldDamage;
    ActualDamage -= ShieldDamage;
  }

  // Remaining damage goes to health
  if (ActualDamage > 0.f) {
    CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.f);
  }

  // Broadcast health change
  const float TotalDamage = Damage * DamageMultiplier;
  OnHealthChanged.Broadcast(this, CurrentHealth, -TotalDamage, InstigatedBy);

  // Check for death
  if (CurrentHealth <= 0.f) {
    bIsDead = true;
    OnDeath.Broadcast(this, InstigatedBy, DamageCauser);
  }
}

// =============================================================================
// PUBLIC API
// =============================================================================

float UHealthComponent::GetHealthPercent() const {
  return (MaxHealth > 0.f) ? CurrentHealth / MaxHealth : 0.f;
}

float UHealthComponent::GetShieldPercent() const {
  return (MaxShield > 0.f) ? CurrentShield / MaxShield : 0.f;
}

void UHealthComponent::Heal(float HealAmount) {
  if (bIsDead || HealAmount <= 0.f) {
    return;
  }

  const float OldHealth = CurrentHealth;
  CurrentHealth = FMath::Min(CurrentHealth + HealAmount, MaxHealth);

  const float ActualHeal = CurrentHealth - OldHealth;
  if (ActualHeal > 0.f) {
    OnHealthChanged.Broadcast(this, CurrentHealth, ActualHeal, nullptr);
  }
}

void UHealthComponent::ResetHealth() {
  CurrentHealth = MaxHealth;
  CurrentShield = MaxShield;
  bIsDead = false;
  OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, nullptr);
}
