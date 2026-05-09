// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimSequenceBase.h"
#include "Gameplay/AmmoTypes.h"
#include "Gameplay/AmmoBonusPickup.h"
#include "Gameplay/ItemDefinition.h"
#include "Engine/Texture2D.h"
#include "EnemyBase.generated.h"

/**
 *  Entrée de drop de munitions.
 *  Le drop n'apparaît que si le joueur possède une arme du type correspondant.
 *  Chaque entrée est tirée indépendamment.
 */
USTRUCT(BlueprintType)
struct FAmmoDropEntry
{
    GENERATED_BODY()

    /**
     *  DataAsset de l'item (DA_Item_Ammo_Pistol, etc.).
     *  Si assigné : type, icône et nom viennent du DA — seuls Chance/Quantité/Durée restent visibles.
     *  Si null    : remplir AmmoType + les champs optionnels ci-dessous manuellement.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop")
    TObjectPtr<UItemDefinition> ItemDefinition = nullptr;

    /** Type de munitions (ignoré si ItemDefinition assigné) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (EditCondition = "ItemDefinition == nullptr", EditConditionHides))
    EAmmoType AmmoType = EAmmoType::Pistol;

    /** Probabilité de drop — 0 = jamais, 1 = toujours */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (ClampMin = 0.f, ClampMax = 1.f))
    float DropChance = 0.3f;

    /** Quantité de munitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (ClampMin = 1, ClampMax = 120))
    int32 AmmoAmount = 12;

    /** Durée de vie avant disparition (secondes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (ClampMin = 5.f, ClampMax = 60.f))
    float Lifetime = 12.f;

    /** Icône manuelle (ignorée si ItemDefinition assigné) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (EditCondition = "ItemDefinition == nullptr", EditConditionHides))
    TObjectPtr<UTexture2D> DropIcon = nullptr;

    /** Nom manuel (ignoré si ItemDefinition assigné — nom auto sinon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (EditCondition = "ItemDefinition == nullptr", EditConditionHides))
    FText DropDisplayName;

    /** BP pickup custom pour le mesh/VFX (optionnel — générique si null) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop")
    TSubclassOf<AAmmoBonusPickup> DropClass;
};

class UHealthComponent;
class AWeaponBase;
class UWidgetComponent;
class UBehaviorTree;
class USoundBase;

/**
 * Zone corporelle touchée qui ouvre la fenêtre de finisher.
 */
UENUM(BlueprintType)
enum class EFinisherZone : uint8
{
  None   UMETA(DisplayName = "None"),
  Head   UMETA(DisplayName = "Head"),
  Arm    UMETA(DisplayName = "Arm"),
  Leg    UMETA(DisplayName = "Leg"),
};

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
  FName WeaponSocket = FName("hand_r");

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

  /** Guard against double-death (e.g. rapid damage in same frame) */
  bool bIsDead = false;

  // ========== STUN ==========

  /** True while the enemy is stunned (can't attack or move) */
  UPROPERTY(BlueprintReadOnly, Category = "Enemy|State")
  bool bIsStunned = false;

  FTimerHandle StunTimer;

  // ========== FINISHER SYSTEM ==========

  /** Zone touchée ouvrant le finisher (None si pas disponible) */
  UPROPERTY(BlueprintReadOnly, Category = "Enemy|Finisher")
  EFinisherZone ActiveFinisherZone = EFinisherZone::None;

  /** Durée de la fenêtre de finisher après le tir (secondes) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Finisher",
            meta = (ClampMin = 0.5f, ClampMax = 10.f))
  float FinisherWindowDuration = 3.f;

  /** Multiplicateur de dégâts de mêlée quand le finisher est disponible */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Finisher",
            meta = (ClampMin = 1.f, ClampMax = 10.f))
  float FinisherDamageMultiplier = 3.f;

  /** Durée du stun appliqué après un finisher réussi */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Finisher",
            meta = (ClampMin = 0.f, ClampMax = 10.f))
  float FinisherStunDuration = 2.5f;

  FTimerHandle FinisherWindowTimer;

  // ========== HIT FLASH ==========

  /** Dynamic material instances for hit flash effect */
  TArray<UMaterialInstanceDynamic *> HitFlashMaterials;

  /** Timer for hit flash decay */
  float HitFlashTimer = 0.f;

  /** Duration of the hit flash in seconds */
  static constexpr float HitFlashDuration = 0.15f;

  // ========== ANIMATIONS ==========

  // ========== AMMO DROPS ==========

  /** Drops de munitions au décès — chaque entrée est tirée indépendamment */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AmmoDrop")
  TArray<FAmmoDropEntry> AmmoDrop;

  /** Animation jouée à la mort (AnimSequence — si null → ragdoll immédiat) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
  UAnimSequenceBase* DeathAnim = nullptr;

  /** Animation jouée quand l'ennemi reçoit des dégâts */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
  UAnimSequenceBase* HitReactAnim = nullptr;

  /** Animation jouée quand l'ennemi attaque (zombies) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
  UAnimSequenceBase* AttackAnim = nullptr;

  // ========== AUDIO ==========

  /** Sound played on death */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio")
  USoundBase *DeathSound = nullptr;

  /** Sound played when taking damage (hit reaction) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio")
  USoundBase *HitSound = nullptr;

  /** Ambient grunt sound (played periodically) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio")
  USoundBase *AmbientSound = nullptr;

  /** Interval between ambient grunts */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio",
            meta = (ClampMin = 1.0, ClampMax = 20.0))
  float AmbientSoundInterval = 5.f;

  /** Timer for ambient sounds */
  float AmbientSoundTimer = 0.f;

  /** Death cleanup timer */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Death")
  float DeathCleanupTime = 8.f;

  FTimerHandle DeathCleanupTimer;

  /** Timer pour déclencher le ragdoll après la death anim */
  FTimerHandle DeathRagdollTimer;

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

  /** Returns kill points for this enemy type. Override in subclasses. */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  virtual int32 GetKillPoints() const { return 100; }

  /** Returns the target player (nullptr if none) */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  APawn *GetTargetPlayer() const { return TargetPlayer; }

  /** Force alert the enemy to a specific location */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  void AlertToLocation(const FVector &Location);

  /**
   * Stun l'ennemi pendant Duration secondes.
   * Pendant le stun : mouvement arrêté, attaques bloquées.
   */
  UFUNCTION(BlueprintCallable, Category = "Enemy")
  void ApplyStun(float Duration);

  /** Retourne true si une fenêtre de finisher est active */
  UFUNCTION(BlueprintCallable, Category = "Enemy|Finisher")
  bool IsVulnerableToFinisher() const { return ActiveFinisherZone != EFinisherZone::None; }

  /** Retourne la zone de finisher active */
  UFUNCTION(BlueprintCallable, Category = "Enemy|Finisher")
  EFinisherZone GetFinisherZone() const { return ActiveFinisherZone; }

  /**
   * Applique un finisher de mêlée (dégâts × FinisherDamageMultiplier + stun).
   * Appelé par RevenantOpsCharacter::MeleeAttackPressed quand bVulnerable.
   */
  void ApplyFinisher(float BaseDamage, AController* DamageInstigator, AActor* Causer);

protected:
  /**
   * Détecte la zone corporelle touchée depuis le nom de l'os.
   * Appelé dans TakeDamage override.
   */
  void HandleBoneHit(const FName& BoneName);

  /** Ouvre la fenêtre de finisher pour la zone donnée */
  void OpenFinisherWindow(EFinisherZone Zone);

  /** Ferme la fenêtre (appelé par timer ou après finisher) */
  void CloseFinisherWindow();

  // Override TakeDamage pour détecter les zones corporelles
  virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                            AController* InInstigator, AActor* DamageCauser) override;

  /** Hook Blueprint : fenêtre de finisher ouverte (afficher icône/effet) */
  UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Finisher")
  void BP_OnFinisherWindowOpened(EFinisherZone Zone);

  /** Hook Blueprint : finisher exécuté */
  UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Finisher")
  void BP_OnFinisherExecuted(EFinisherZone Zone);

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
