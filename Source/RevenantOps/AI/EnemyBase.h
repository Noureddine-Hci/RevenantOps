// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UHealthComponent;
class AWeaponBase;
class UWidgetComponent;
class UBehaviorTree;

/**
 *  Enemy behavior profile - determines combat style
 */
UENUM(BlueprintType)
enum class EEnemyBehavior : uint8 {
  Aggressive UMETA(DisplayName = "Aggressive - Rushes the player"),
  Defensive UMETA(DisplayName = "Defensive - Uses cover, peeks"),
  Flanker UMETA(DisplayName = "Flanker - Circles around the player"),
  Sniper UMETA(DisplayName = "Sniper - Stays far, high accuracy"),
  Patrol UMETA(DisplayName = "Patrol - Follows patrol route")
};

/**
 *  Enemy alert state
 */
UENUM(BlueprintType)
enum class EEnemyAlertState : uint8 {
  Idle UMETA(DisplayName = "Idle - Unaware"),
  Suspicious UMETA(DisplayName = "Suspicious - Investigating"),
  Alert UMETA(DisplayName = "Alert - In combat"),
  Searching UMETA(DisplayName = "Searching - Lost sight of player")
};

/**
 *  Delegate when enemy dies
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyBaseDied, AEnemyBase *,
                                              Enemy, AController *,
                                              KilledBy);

/**
 *  Base enemy class for TPS gameplay.
 *  Uses AIPerception for detection and supports weapon-based combat.
 *  Configure behavior profile, weapon class, and patrol points in Blueprint.
 */
UCLASS(abstract, Blueprintable)
class AEnemyBase : public ACharacter {
  GENERATED_BODY()

public:
  AEnemyBase();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ========== COMPONENTS ==========

  /** Health component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UHealthComponent *HealthComp;

  /** Life bar widget above head */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UWidgetComponent *LifeBarWidget;

  // ========== IDENTITY ==========

  /** Enemy display name */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Identity")
  FText EnemyName;

  /** Behavior profile */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Identity")
  EEnemyBehavior BehaviorProfile = EEnemyBehavior::Aggressive;

  /** Current alert state */
  UPROPERTY(BlueprintReadOnly, Category = "Enemy|State")
  EEnemyAlertState AlertState = EEnemyAlertState::Idle;

  // ========== COMBAT ==========

  /** Weapon class to spawn and equip */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
  TSubclassOf<AWeaponBase> WeaponClass;

  /** Spawned weapon instance */
  UPROPERTY(BlueprintReadOnly, Category = "Enemy|Combat")
  AWeaponBase *EquippedWeapon = nullptr;

  /** Socket to attach weapon */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
  FName WeaponSocket = FName("WeaponSocket");

  /** Ideal engagement distance (tries to stay around this range) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat",
            meta = (ClampMin = 100, ClampMax = 10000))
  float IdealEngagementRange = 1500.f;

  /** Maximum engagement distance (won't shoot beyond this) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat",
            meta = (ClampMin = 100, ClampMax = 20000))
  float MaxEngagementRange = 5000.f;

  /** Accuracy modifier (0 = terrible, 1 = perfect aim) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat",
            meta = (ClampMin = 0.0, ClampMax = 1.0))
  float Accuracy = 0.5f;

  /** Time between bursts of fire (seconds) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat",
            meta = (ClampMin = 0.1, ClampMax = 10.0))
  float FireBurstInterval = 2.0f;

  /** Number of shots per burst */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat",
            meta = (ClampMin = 1, ClampMax = 30))
  int32 ShotsPerBurst = 5;

  /** Time to aim before first shot (reaction time) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat",
            meta = (ClampMin = 0.0, ClampMax = 5.0))
  float ReactionTime = 0.8f;

  // ========== PERCEPTION ==========

  /** Sight range */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception",
            meta = (ClampMin = 100, ClampMax = 10000))
  float SightRange = 3000.f;

  /** Sight cone half-angle in degrees */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception",
            meta = (ClampMin = 10, ClampMax = 180))
  float SightHalfAngle = 60.f;

  /** Hearing range */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception",
            meta = (ClampMin = 0, ClampMax = 5000))
  float HearingRange = 1500.f;

  /** Time to lose sight of player before going to Searching state */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception",
            meta = (ClampMin = 0.5, ClampMax = 30.0))
  float LoseSightTime = 5.f;

  /** Time spent searching before returning to Idle */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception",
            meta = (ClampMin = 1.0, ClampMax = 60.0))
  float SearchDuration = 10.f;

  // ========== PATROL ==========

  /** Patrol waypoints (in world space, set in level) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol")
  TArray<FVector> PatrolPoints;

  /** Current patrol point index */
  int32 CurrentPatrolIndex = 0;

  /** Wait time at each patrol point */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol",
            meta = (ClampMin = 0, ClampMax = 30))
  float PatrolWaitTime = 3.f;

  // ========== INTERNAL STATE ==========

  /** Reference to the detected player */
  UPROPERTY()
  APawn *TargetPlayer = nullptr;

  /** Last known location of the player */
  FVector LastKnownPlayerLocation = FVector::ZeroVector;

  /** Time since player was last seen */
  float TimeSinceLastSeen = 0.f;

  /** Timer for fire bursts */
  float FireBurstTimer = 0.f;

  /** Shots fired in current burst */
  int32 CurrentBurstShots = 0;

  /** True if currently in a fire burst */
  bool bIsFiring = false;

  /** Reaction timer (delay before first engagement) */
  float CurrentReactionTimer = 0.f;

  /** Has reacted to seeing the player */
  bool bHasReacted = false;

  /** Death cleanup timer */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Death")
  float DeathCleanupTime = 8.f;

  FTimerHandle DeathCleanupTimer;

public:
  // ========== EVENTS ==========

  UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
  FOnEnemyBaseDied OnEnemyDied;

  // ========== PUBLIC API ==========

  /** Returns the current alert state */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  EEnemyAlertState GetAlertState() const { return AlertState; }

  /** Returns the health component */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  UHealthComponent *GetHealthComp() const { return HealthComp; }

  /** Returns the equipped weapon */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  AWeaponBase *GetEquippedWeapon() const { return EquippedWeapon; }

  /** Returns the target player (nullptr if none) */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  APawn *GetTargetPlayer() const { return TargetPlayer; }

  /** Force alert the enemy to a specific location */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  void AlertToLocation(const FVector &Location);

  /** Notify from squad that player is at a location */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  void ReceiveSquadAlert(const FVector &PlayerLocation, AEnemyBase *Alerter);

protected:
  // ========== AI LOGIC ==========

  /** Checks if the player is within sight cone and range */
  bool CanSeePlayer() const;

  /** Updates perception (sight check, alert state transitions) */
  void UpdatePerception(float DeltaTime);

  /** Updates combat behavior based on alert state */
  void UpdateCombat(float DeltaTime);

  /** Updates patrol behavior */
  void UpdatePatrol();

  /** Fires at the target player with accuracy spread */
  void FireAtPlayer();

  /** Moves towards the ideal engagement position */
  void MoveToEngagementPosition();

  /** Called when health component broadcasts death */
  UFUNCTION()
  void HandleDeath(UHealthComponent *HealthComponent,
                   const AController *InstigatedBy, AActor *DamageCauser);

  /** Called when health changes */
  UFUNCTION()
  void HandleDamage(UHealthComponent *HealthComponent, float Health,
                    float HealthDelta, const AController *InstigatedBy);

  /** Cleanup after death */
  void DeathCleanup();

  /** Spawns and equips the weapon */
  void SpawnWeapon();

  // ========== BLUEPRINT HOOKS ==========

  /** Called when alert state changes */
  UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events",
            meta = (DisplayName = "On Alert State Changed"))
  void BP_OnAlertStateChanged(EEnemyAlertState NewState);

  /** Called on death for effects */
  UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events",
            meta = (DisplayName = "On Death"))
  void BP_OnDeath();

  /** Called when damage is received */
  UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events",
            meta = (DisplayName = "On Damage Received"))
  void BP_OnDamageReceived(float Damage, const FVector &HitDirection);
};
