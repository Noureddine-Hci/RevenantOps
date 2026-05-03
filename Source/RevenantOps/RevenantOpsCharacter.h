// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Gameplay/InventoryItem.h"
#include "Gameplay/TalentDefinition.h"
#include "RevenantOpsCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAnimMontage;
class AWeaponBase;
class UHealthComponent;
class AAmmoBonusPickup;
class AHealthPickup;
// IPickupInterface — utilisee dans InteractPressed via Execute_
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  RevenantOps base third-person character.
 *  Features: smooth sprint, crouch, slide, dodge, stamina, weapon system.
 */
UCLASS(abstract)
class ARevenantOpsCharacter : public ACharacter {
  GENERATED_BODY()

  /** Camera boom positioning the camera behind the character */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  USpringArmComponent *CameraBoom;

  /** Follow camera */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  UCameraComponent *FollowCamera;

protected:
  // ========== INPUT ACTIONS ==========

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *JumpAction;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *MoveAction;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *LookAction;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction *MouseLookAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *SprintAction;

  /** Crouch Input Action (toggle) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *CrouchAction;

  /** Dodge/Roll Input Action */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *DodgeAction;

  /** Fire Input Action (hold for auto) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *FireAction;

  /** ADS / Aim Input Action (hold) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *AimAction;

  /** Reload Input Action */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *ReloadAction;

  /** Switch Weapon Input Action */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *SwitchWeaponAction;

  /** Interact / Ramasser (touche E) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
            meta = (AllowPrivateAccess = "true"))
  UInputAction *InteractAction;

  // ========== MOVEMENT SPEEDS ==========

  /** Normal walk speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed",
            meta = (ClampMin = 0, ClampMax = 2000))
  float WalkSpeed = 500.f;

  /** Sprint speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed",
            meta = (ClampMin = 0, ClampMax = 2000))
  float SprintSpeed = 900.f;

  /** Crouch walk speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed",
            meta = (ClampMin = 0, ClampMax = 2000))
  float CrouchMoveSpeed = 250.f;

  /** Speed interpolation rate (higher = snappier transitions) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed",
            meta = (ClampMin = 1.0, ClampMax = 30.0))
  float SpeedInterpRate = 8.f;

  // ========== SPRINT ==========

  UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint")
  bool bIsSprinting = false;

  bool bWantsToSprint = false;

  // ========== SLIDE ==========

  UPROPERTY(BlueprintReadOnly, Category = "Movement|Slide")
  bool bIsSliding = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 3000))
  float SlideBoostSpeed = 1200.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 10))
  float SlideGroundFriction = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 5000))
  float SlideBrakingDeceleration = 800.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 1000))
  float SlideMinSpeed = 200.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0.1, ClampMax = 3.0))
  float SlideMaxDuration = 1.2f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 100))
  float SlideCost = 20.f;

  FTimerHandle SlideTimerHandle;
  float DefaultGroundFriction = 8.f;
  float DefaultBrakingDeceleration = 2000.f;

  // ========== DODGE / ROLL ==========

  UPROPERTY(BlueprintReadOnly, Category = "Movement|Dodge")
  bool bIsDodging = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge")
  UAnimMontage *DodgeMontage;

  // ========== ANIMATIONS ==========

  /** Montage joué à la mort du joueur (chute en avant) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
  UAnimMontage* DeathMontage = nullptr;

  /** Montage joué en début de sprint (optionnel — transitions upper body) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
  UAnimMontage* SprintStartMontage = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge",
            meta = (ClampMin = 0, ClampMax = 2000))
  float DodgeLaunchForce = 800.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge",
            meta = (ClampMin = 0, ClampMax = 3))
  float DodgeCooldown = 0.6f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge",
            meta = (ClampMin = 0, ClampMax = 100))
  float DodgeCost = 25.f;

  float LastDodgeTime = -100.f;
  FTimerHandle DodgeEndTimerHandle;
  FOnMontageEnded OnDodgeMontageEnded;

  // ========== STAMINA ==========

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 200))
  float MaxStamina = 100.f;

  UPROPERTY(BlueprintReadOnly, Category = "Movement|Stamina")
  float CurrentStamina = 100.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 100))
  float SprintStaminaDrain = 15.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 100))
  float StaminaRegenRate = 20.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 5))
  float StaminaRegenDelay = 1.0f;

  float LastStaminaDrainTime = -100.f;

  UPROPERTY(BlueprintReadOnly, Category = "Movement|Stamina")
  bool bStaminaDepleted = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 100))
  float StaminaRecoveryThreshold = 20.f;

  // ========== CAMERA ==========

  /** Default FOV */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
            meta = (ClampMin = 60, ClampMax = 130))
  float DefaultFOV = 90.f;

  /** Sprint FOV */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
            meta = (ClampMin = 60, ClampMax = 130))
  float SprintFOV = 100.f;

  /** FOV interpolation speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
            meta = (ClampMin = 1.0, ClampMax = 20.0))
  float FOVInterpSpeed = 6.f;

  float TargetSpeed = 500.f;

  // ========== WEAPON SYSTEM ==========

  /** Weapon classes to spawn on begin play (loadout) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon",
            meta = (AllowPrivateAccess = "true"))
  TArray<TSubclassOf<AWeaponBase>> DefaultWeaponClasses;

  /** Socket name to attach weapons to the character mesh */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
  FName WeaponAttachSocket = FName("hand_r");

  /** Currently equipped weapon */
  UPROPERTY(BlueprintReadOnly, Category = "Weapon")
  AWeaponBase *CurrentWeapon = nullptr;

  /** All weapons in inventory */
  UPROPERTY(BlueprintReadOnly, Category = "Weapon")
  TArray<AWeaponBase *> WeaponInventory;

  /** Index of the current weapon in inventory */
  int32 CurrentWeaponIndex = 0;

  /** Is the character currently aiming */
  UPROPERTY(BlueprintReadOnly, Category = "Weapon")
  bool bIsAiming = false;

  /** True when the character has at least one weapon equipped — used by ABP */
  UPROPERTY(BlueprintReadOnly, Category = "Animation")
  bool bIsArmed = false;

  /** Aim pitch delta (control vs actor), clamped [-90, 90] — used by AimOffset in ABP */
  UPROPERTY(BlueprintReadOnly, Category = "Animation")
  float AimPitch = 0.f;

  /** Aim yaw delta (control vs actor), clamped [-180, 180] — used by AimOffset in ABP */
  UPROPERTY(BlueprintReadOnly, Category = "Animation")
  float AimYaw = 0.f;

  /** Movement direction relative to actor (degrees, -180 to 180) — used by armed blendspace */
  UPROPERTY(BlueprintReadOnly, Category = "Animation")
  float MovementDirection = 0.f;

public:
  // ── Perks personnage (override dans chaque BP enfant) ────────────────────

  /** Multiplicateur de vitesse de rechargement (1.0 = normal, 1.5 = 50% plus vite) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Perks",
            meta = (ClampMin = 0.5f, ClampMax = 3.f))
  float ReloadSpeedMultiplier = 1.0f;

  /** Réduction des dégâts reçus en % (0 = aucune, 0.25 = 25% de réduction) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Perks",
            meta = (ClampMin = 0.f, ClampMax = 0.75f))
  float DamageResistance = 0.0f;

  /** Multiplicateur de munitions en réserve (1.0 = normal, 1.5 = 50% de plus) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Perks",
            meta = (ClampMin = 0.5f, ClampMax = 3.f))
  float AmmoCapacityMultiplier = 1.0f;

  /** Bonus de vitesse de déplacement (ajouté à WalkSpeed) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Perks",
            meta = (ClampMin = 0.f, ClampMax = 300.f))
  float MoveSpeedBonus = 0.0f;

  /** Description courte des perks pour l'écran de sélection */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Perks")
  FText PerkDescription;

  // ── Système de talents ────────────────────────────────────────────────────

  /** Talents actifs sur ce personnage (assignés depuis FCharacterInfo ou en debug) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Talents")
  TArray<TObjectPtr<UTalentDefinition>> AssignedTalents;

  /** Applique les bonus de tous les talents assignés (appelé dans BeginPlay) */
  UFUNCTION(BlueprintCallable, Category = "Character|Talents")
  void ApplyTalents();

  /** Cached health component for damage feedback */
  UPROPERTY()
  UHealthComponent *HealthComp = nullptr;

  /**
   *  Pickup actif en zone (munitions, soins...) — interroge via IPickupInterface.
   *  Raw ptr intentionnel (pas de UPROPERTY pour eviter CDO crash).
   */
  AActor* PendingInteractable = nullptr;

  /** Weapon pickup en zone — raw ptr */
  class AWeaponPickup *PendingWeaponPickup = nullptr;

  // ========== INVENTORY (RE5-style, 9 slots) ==========

  /** 9-slot inventory shared between items and weapons */
  UPROPERTY(BlueprintReadOnly, Category = "Inventory")
  TArray<FInventoryItem> Inventory;

public:
  ARevenantOpsCharacter();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

  // ========== INPUT HANDLERS ==========

  void Move(const FInputActionValue &Value);
  void Look(const FInputActionValue &Value);
  void StartSprint();
  void StopSprint();
  void CrouchPressed();
  void DodgePressed();
  void FirePressed();
  void FireReleased();
  void AimPressed();
  void AimReleased();
  void ReloadPressed();
  void SwitchWeaponPressed();
  void InteractPressed();

  // ========== LOCOMOTION LOGIC ==========

  void UpdateMovementSpeed(float DeltaTime);
  void UpdateStamina(float DeltaTime);
  void UpdateCameraFOV(float DeltaTime);
  /** Updates AimPitch, AimYaw, MovementDirection each tick for ABP */
  void UpdateAnimationValues();
  bool ConsumeStamina(float Amount);
  void StartSlide();
  void EndSlide();
  void StartDodge();
  void DodgeMontageEnded(UAnimMontage *Montage, bool bInterrupted);

  /** Called when health changes — triggers camera shake and damage direction */
  UFUNCTION()
  void OnDamageReceived(UHealthComponent *Comp, float Health, float HealthDelta,
                        const AController *InstigatedBy);

public:
  // ========== WEAPON LOGIC ==========

  /** Sets weapon classes for loadout (call before SpawnDefaultWeapons) */
  void SetDefaultWeaponClasses(
      const TArray<TSubclassOf<AWeaponBase>> &NewClasses);

  /** Destroys existing weapons and respawns from DefaultWeaponClasses */
  void SpawnDefaultWeapons();

protected:
  /** Equips a weapon from inventory by index */
  void EquipWeapon(int32 Index);

  /** Attaches current weapon to the character mesh */
  void AttachWeaponToSocket(AWeaponBase *Weapon);

public:
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoMove(float Right, float Forward);

  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoLook(float Yaw, float Pitch);

  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoJumpStart();

  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoJumpEnd();

  UFUNCTION(BlueprintCallable, Category = "Movement|Stamina")
  float GetStaminaPercent() const;

  UFUNCTION(BlueprintCallable, Category = "Movement")
  bool IsSprinting() const { return bIsSprinting; }

  UFUNCTION(BlueprintCallable, Category = "Movement")
  bool IsSliding() const { return bIsSliding; }

  UFUNCTION(BlueprintCallable, Category = "Movement")
  bool IsDodging() const { return bIsDodging; }

  UFUNCTION(BlueprintCallable, Category = "Weapon")
  bool IsAiming() const { return bIsAiming; }

  /** Returns the currently equipped weapon */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  AWeaponBase *GetCurrentWeapon() const { return CurrentWeapon; }

  /** Returns a copy of the 9-slot inventory */
  UFUNCTION(BlueprintCallable, Category = "Inventory")
  TArray<FInventoryItem> GetInventoryItems() const { return Inventory; }

  /** Adds an item to the first empty slot; returns false if inventory is full */
  UFUNCTION(BlueprintCallable, Category = "Inventory")
  bool AddItemToInventory(const FInventoryItem& Item);

  /** Uses item at SlotIndex (applies heal/time bonus, removes consumable) */
  UFUNCTION(BlueprintCallable, Category = "Inventory")
  void UseInventoryItem(int32 SlotIndex);

public:
  /** Enregistre n'importe quel pickup IPickupInterface actif */
  void SetPendingPickup(AActor* Pickup) { PendingInteractable = Pickup; }

  /** Efface le pickup actif (evite l'ambiguite sur nullptr) */
  void ClearPendingPickup() { PendingInteractable = nullptr; }

  /** Alias pour HealthPickup — identique a SetPendingPickup */
  void SetPendingHealthPickup(AActor* Pickup) { PendingInteractable = Pickup; }

  /** Enregistre le weapon pickup actif */
  void SetPendingWeaponPickup(AWeaponPickup* Pickup) { PendingWeaponPickup = Pickup; }

  /** Ajoute une arme déjà spawnée à l'inventaire et l'équipe — appelé par WeaponPickup */
  void AddAndEquipWeapon(AWeaponBase* NewWeapon);

  /** Retourne le nombre d'armes dans l'inventaire */
  int32 GetWeaponCount() const { return WeaponInventory.Num(); }

  /** Retourne toutes les armes de l'inventaire (lecture seule) */
  const TArray<AWeaponBase*>& GetWeaponInventory() const { return WeaponInventory; }

  /** Affiche le popup RE5 sur le HUD (icone + [E] + nom) */
  void ShowPickupPrompt(UTexture2D* Icon, const FText& Name, int32 Qty);

  /** Cache le popup RE5 */
  void HidePickupPrompt();

  FORCEINLINE class USpringArmComponent *GetCameraBoom() const {
    return CameraBoom;
  }
  FORCEINLINE class UCameraComponent *GetFollowCamera() const {
    return FollowCamera;
  }
};
