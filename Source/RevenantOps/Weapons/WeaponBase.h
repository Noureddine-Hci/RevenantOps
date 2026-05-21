// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Gameplay/AmmoTypes.h"
#include "CrosshairTypes.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UAnimMontage;
class UAnimSequenceBase;
class USoundBase;
class UNiagaraSystem;

/**
 *  Weapon fire mode
 */
UENUM(BlueprintType)
enum class EWeaponFireMode : uint8 {
  SemiAuto UMETA(DisplayName = "Semi-Auto"),
  FullAuto UMETA(DisplayName = "Full-Auto"),
  Burst UMETA(DisplayName = "Burst")
};

/**
 *  Weapon category
 */
UENUM(BlueprintType)
enum class EWeaponCategory : uint8 {
  Pistol UMETA(DisplayName = "Pistol"),
  SMG UMETA(DisplayName = "SMG"),
  AssaultRifle UMETA(DisplayName = "Assault Rifle"),
  Shotgun UMETA(DisplayName = "Shotgun"),
  Sniper UMETA(DisplayName = "Sniper"),
  LMG UMETA(DisplayName = "LMG"),
  Launcher UMETA(DisplayName = "Launcher"),
  Melee UMETA(DisplayName = "Melee")
};

/**
 *  Weapon state
 */
UENUM(BlueprintType)
enum class EWeaponState : uint8 {
  Idle,
  Firing,
  Reloading,
  Equipping,
  Unequipping
};

/**
 *  Delegate fired when ammo changes (for UI binding)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32,
                                              CurrentAmmo, int32, MaxAmmo);

/**
 *  Delegate fired when weapon state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponStateChanged,
                                            EWeaponState, NewState);

/**
 *  Base weapon class for RevenantOps.
 *  Handles: hitscan firing, recoil, spread, reload, ADS, fire modes.
 *  Extend in Blueprint to set mesh, sounds, VFX, and per-weapon tuning.
 */
UCLASS(abstract, Blueprintable)
class AWeaponBase : public AActor {
  GENERATED_BODY()

public:
  AWeaponBase();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ========== COMPONENTS ==========

  /** Weapon skeletal mesh */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USkeletalMeshComponent *WeaponMesh;

  /** Placeholder static mesh — visible tant qu'aucun SKM n'est assigné */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *WeaponMeshSM;

  // ========== WEAPON IDENTITY ==========

  /** Display name */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Identity")
  FText WeaponName;

  /** Category */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Identity")
  EWeaponCategory WeaponCategory = EWeaponCategory::AssaultRifle;

  /** Icone affichee dans l'inventaire (optionnel) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Identity")
  UTexture2D* WeaponIcon = nullptr;

  /** DataTable row handle — set in BP Defaults to {DT_WeaponStats, "RowName"} */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Identity")
  FDataTableRowHandle WeaponDataRow;

  /** Type de munitions — utilisé pour les drops ennemis */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Identity")
  EAmmoType WeaponAmmoType = EAmmoType::Pistol;

  // ========== FIRING ==========

  /** Fire mode */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firing")
  EWeaponFireMode FireMode = EWeaponFireMode::FullAuto;

  /** Rounds per minute */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Firing",
            meta = (ClampMin = 30, ClampMax = 1800))
  float FireRate = 600.f;

  /** Number of pellets per shot (> 1 for shotguns) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Firing",
            meta = (ClampMin = 1, ClampMax = 20))
  int32 PelletsPerShot = 1;

  /** Number of rounds per burst (only used in Burst mode) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Firing",
            meta = (ClampMin = 2, ClampMax = 10))
  int32 BurstCount = 3;

  /** Maximum hitscan range in cm */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Firing",
            meta = (ClampMin = 100, ClampMax = 100000))
  float MaxRange = 10000.f;

  /** Base damage per hit */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Damage",
            meta = (ClampMin = 0, ClampMax = 500))
  float BaseDamage = 20.f;

  /** Headshot damage multiplier */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Damage",
            meta = (ClampMin = 1.0, ClampMax = 10.0))
  float HeadshotMultiplier = 2.0f;

  /** Damage falloff start distance (cm) - full damage up to this range */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Damage",
            meta = (ClampMin = 0, ClampMax = 50000))
  float DamageFalloffStart = 2000.f;

  /** Damage falloff end distance (cm) - minimum damage beyond this range */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Damage",
            meta = (ClampMin = 0, ClampMax = 100000))
  float DamageFalloffEnd = 5000.f;

  /** Minimum damage multiplier at max falloff */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Damage",
            meta = (ClampMin = 0.0, ClampMax = 1.0))
  float MinDamageMultiplier = 0.4f;

  // ========== AMMO ==========

  /** Magazine capacity */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo",
            meta = (ClampMin = 1, ClampMax = 200))
  int32 MagazineSize = 30;

  /** Current ammo in magazine */
  UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
  int32 CurrentAmmo = 30;

  /** Total reserve ammo */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo",
            meta = (ClampMin = 0, ClampMax = 999))
  int32 MaxReserveAmmo = 120;

  /** Current reserve ammo */
  UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
  int32 CurrentReserveAmmo = 120;

  /** Reload time in seconds */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo",
            meta = (ClampMin = 0.1, ClampMax = 10.0))
  float ReloadTime = 2.0f;

  // ========== RECOIL ==========

  /** Vertical recoil per shot (pitch, in degrees) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Recoil",
            meta = (ClampMin = 0, ClampMax = 10))
  float VerticalRecoil = 0.6f;

  /** Horizontal recoil range per shot (yaw, random between -X and +X) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Recoil",
            meta = (ClampMin = 0, ClampMax = 5))
  float HorizontalRecoilRange = 0.3f;

  /** Recoil recovery speed (how fast the camera returns to center) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Recoil",
            meta = (ClampMin = 1.0, ClampMax = 30.0))
  float RecoilRecoverySpeed = 8.f;

  /** ADS recoil multiplier (usually < 1 for more stable aiming) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Recoil",
            meta = (ClampMin = 0.1, ClampMax = 2.0))
  float ADSRecoilMultiplier = 0.6f;

  /** Accumulated recoil to recover from */
  FVector2D AccumulatedRecoil = FVector2D::ZeroVector;

  // ========== SPREAD ==========

  /** Base spread angle (degrees) when hip-firing */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Spread",
            meta = (ClampMin = 0, ClampMax = 15))
  float BaseSpread = 2.0f;

  /** Maximum spread angle (degrees) after sustained fire */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Spread",
            meta = (ClampMin = 0, ClampMax = 20))
  float MaxSpread = 6.0f;

  /** Spread increase per shot */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Spread",
            meta = (ClampMin = 0, ClampMax = 5))
  float SpreadPerShot = 0.5f;

  /** Spread recovery speed (degrees per second) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Spread",
            meta = (ClampMin = 1.0, ClampMax = 30.0))
  float SpreadRecoverySpeed = 5.f;

  /** ADS spread multiplier (tighter when aiming) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Spread",
            meta = (ClampMin = 0.0, ClampMax = 1.0))
  float ADSSpreadMultiplier = 0.3f;

  /** Current dynamic spread */
  float CurrentSpread = 0.f;

  // ========== CROSSHAIR ==========
  // Ces valeurs sont lues depuis DT_WeaponStats via ApplyWeaponDataRow().
  // Ne pas les modifier dans le BP — modifier la DataTable à la place.

  /** Style de réticule — chargé depuis DT_WeaponStats */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Crosshair")
  ECrosshairStyle CrosshairStyle = ECrosshairStyle::Cross;

  /** Texture de scope ADS (sniper) — chargée depuis DT_WeaponStats */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Crosshair")
  TObjectPtr<UTexture2D> ScopeOverlayTexture = nullptr;

  /** Multiplicateur FOV scope — chargé depuis DT_WeaponStats */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Crosshair")
  float ScopeFOVMultiplier = 0.4f;

  // ========== ADS (Aim Down Sights) ==========

  /** ADS FOV (zoomed in) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|ADS",
            meta = (ClampMin = 20, ClampMax = 90))
  float ADSFOV = 65.f;

  /** ADS transition speed */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|ADS",
            meta = (ClampMin = 1.0, ClampMax = 30.0))
  float ADSInterpSpeed = 12.f;

  /** ADS movement speed multiplier */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|ADS",
            meta = (ClampMin = 0.1, ClampMax = 1.0))
  float ADSMoveSpeedMultiplier = 0.6f;

  // ========== ANIMATIONS ==========

  /** Weapon fire animation (played on weapon mesh — sequence, no AnimBP needed) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
            Category = "Weapon|Animation")
  UAnimSequenceBase *FireMontage;

  /** Character fire animation (played on character mesh) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
            Category = "Weapon|Animation")
  UAnimMontage *CharacterFireMontage;

  /** Character reload animation */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
            Category = "Weapon|Animation")
  UAnimMontage *ReloadMontage;

  /** Weapon-side reload animation (played on weapon mesh — sequence, no AnimBP needed) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
            Category = "Weapon|Animation")
  UAnimSequenceBase *WeaponReloadAnim;

  /** Character equip animation */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
            Category = "Weapon|Animation")
  UAnimMontage *EquipMontage;

  /** Socket sur le weapon mesh où la main gauche doit se positionner (Left Hand IK) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
            Category = "Weapon|Animation")
  FName LeftHandGripSocket = FName("LeftHandGrip");

  // ========== AUDIO ==========

  /** Sound played on each shot */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Audio")
  USoundBase *FireSound = nullptr;

  /** Sound played when reload starts */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Audio")
  USoundBase *ReloadSound = nullptr;

  /** Sound played on empty magazine click */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Audio")
  USoundBase *EmptySound = nullptr;

  // ========== VFX ==========

  /** Muzzle flash Niagara system spawned at muzzle on each shot */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|VFX")
  UNiagaraSystem *MuzzleFlashVFX = nullptr;

  /** Impact Niagara system spawned at hit location */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|VFX")
  UNiagaraSystem *ImpactVFX = nullptr;

  /** Blood impact Niagara system (used when hitting enemies) */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|VFX")
  UNiagaraSystem *BloodImpactVFX = nullptr;

  // ========== SOCKETS ==========

  /** Muzzle socket name on weapon mesh */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Sockets")
  FName MuzzleSocketName = FName("Muzzle");

  /** Socket name to attach weapon to character mesh */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Sockets")
  FName AttachSocketName = FName("hand_r");

  /** Position/rotation offset appliqué après l'attach au socket — ajuster par BP */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Sockets")
  FTransform WeaponAttachOffset = FTransform::Identity;

  /**
   *  Socket d'holster — où l'arme est attachée quand elle n'est PAS équipée.
   *  Ex: "HolsterBack" pour fusil, "HolsterHip" pour pistolet.
   *  Si vide ou socket inexistant, l'arme est cachée.
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Sockets")
  FName HolsterSocketName = NAME_None;

  /** Offset position/rotation appliqué quand l'arme est dans son holster */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Sockets")
  FTransform HolsterOffset = FTransform::Identity;

  // ========== STATE ==========

  /** Current weapon state */
  UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
  EWeaponState CurrentState = EWeaponState::Idle;

  /** Is the player currently aiming down sights */
  UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
  bool bIsADS = false;

  /** Is the fire input currently held */
  bool bWantsToFire = false;

  /** Time of last shot fired */
  float LastFireTime = -100.f;

  /** Burst shots remaining */
  int32 BurstShotsRemaining = 0;

  /** Reload timer */
  FTimerHandle ReloadTimerHandle;

  /** Time when current reload started (for progress calculation) */
  float ReloadStartTime = 0.f;

  /** Cached reference to the owning character's controller */
  UPROPERTY()
  AController *OwnerController = nullptr;

  /** Cached reference to the owning character */
  UPROPERTY()
  APawn *OwnerPawn = nullptr;

public:
  // ========== EVENTS ==========

  UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
  FOnAmmoChanged OnAmmoChanged;

  UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
  FOnWeaponStateChanged OnWeaponStateChanged;

  // ========== PUBLIC API ==========

  /** Called by the character to start firing */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  void StartFire();

  /** Called by the character to stop firing */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  void StopFire();

  /** Called by the character to reload */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  void StartReload();

  /** Called by the character to enter ADS */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  void StartADS();

  /** Called by the character to exit ADS */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  void StopADS();

  /** Sets the owning pawn (called when equipping) */
  void SetOwnerPawn(APawn *NewOwner);

  /** Gets the weapon mesh */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  USkeletalMeshComponent *GetWeaponMesh() const { return WeaponMesh; }

  /** Retourne le transform monde du socket LeftHandGrip (pour le Left Hand IK dans l'ABP) */
  UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
  FTransform GetLeftHandGripTransform() const
  {
    if (WeaponMesh && WeaponMesh->DoesSocketExist(LeftHandGripSocket))
      return WeaponMesh->GetSocketTransform(LeftHandGripSocket);
    return FTransform::Identity;
  }

  /** Gets current spread in degrees */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  float GetCurrentSpread() const;

  /** Style de réticule de cette arme */
  UFUNCTION(BlueprintCallable, Category = "Weapon|Crosshair")
  ECrosshairStyle GetCrosshairStyle() const { return CrosshairStyle; }

  /** Texture de scope (nullptr si pas de scope) */
  UFUNCTION(BlueprintCallable, Category = "Weapon|Crosshair")
  UTexture2D* GetScopeOverlayTexture() const { return ScopeOverlayTexture; }

  /** True si cette arme a un scope (sniper) */
  UFUNCTION(BlueprintCallable, Category = "Weapon|Crosshair")
  bool HasScope() const { return ScopeOverlayTexture != nullptr; }

  /** Gets ADS alpha (0 = hip, 1 = fully aimed) for blending */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  float GetADSAlpha() const { return ADSAlpha; }

  /** Returns true if the weapon can fire right now */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  bool CanFire() const;

  /** Returns true if the weapon can reload right now */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  bool CanReload() const;

  /** Gets current ammo in magazine */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  int32 GetCurrentAmmo() const { return CurrentAmmo; }

  /** Gets current reserve ammo (RE5 : lit depuis l'inventaire si owner est un ARevenantOpsCharacter) */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  int32 GetCurrentReserveAmmo() const;

  /** Gets weapon display name */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  FText GetWeaponName() const { return WeaponName; }

  /** Gets weapon icon texture (for inventory) */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  UTexture2D* GetWeaponIcon() const { return WeaponIcon; }

  /** Gets base damage */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  float GetBaseDamage() const { return BaseDamage; }

  /** Gets fire rate in RPM */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  float GetFireRate() const { return FireRate; }

  /** Gets magazine size */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  int32 GetMagazineSize() const { return MagazineSize; }

  /** Gets reload time in seconds */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  float GetReloadTime() const { return ReloadTime; }

  /** Returns the current weapon state */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  EWeaponState GetCurrentState() const { return CurrentState; }

  /** Returns the equip montage (used by character to play on weapon switch) */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  UAnimMontage* GetEquipMontage() const { return EquipMontage; }

  /** Returns reload progress (0..1). Valid only when state == Reloading. */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  float GetReloadProgress() const;

  /** Gets ADS field of view */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  float GetADSFOV() const { return ADSFOV; }

  int32 GetMaxReserveAmmo() const { return MaxReserveAmmo; }
  EAmmoType GetWeaponAmmoType() const { return WeaponAmmoType; }
  EWeaponCategory GetWeaponCategory() const { return WeaponCategory; }
  FTransform GetWeaponAttachOffset() const { return WeaponAttachOffset; }

  /** Socket d'holster (NAME_None si arme à cacher quand non équipée) */
  FName GetHolsterSocketName() const { return HolsterSocketName; }

  /** Offset à appliquer dans l'holster */
  FTransform GetHolsterOffset() const { return HolsterOffset; }
  void SetMaxReserveAmmo(int32 NewMax)
  {
    MaxReserveAmmo = FMath::Max(1, NewMax);
    CurrentReserveAmmo = FMath::Min(CurrentReserveAmmo, MaxReserveAmmo);
  }

  /** Adds ammo to reserve (from pickups) */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  void AddReserveAmmo(int32 Amount) {
    CurrentReserveAmmo = FMath::Min(CurrentReserveAmmo + Amount, MaxReserveAmmo);
    OnAmmoChanged.Broadcast(CurrentAmmo, MagazineSize);
  }

  /** Ajoute directement des munitions dans le chargeur (combine inventaire → arme). */
  UFUNCTION(BlueprintCallable, Category = "Weapon")
  int32 AddDirectlyToMagazine(int32 Amount)
  {
    const int32 Space    = MagazineSize - CurrentAmmo;
    const int32 Added    = FMath::Min(Amount, Space);
    CurrentAmmo += Added;
    OnAmmoChanged.Broadcast(CurrentAmmo, MagazineSize);
    return Added; // retourne la quantité réellement ajoutée
  }

  /** Retourne les munitions actuelles dans le chargeur */
  int32 GetCurrentMagazineAmmo() const { return CurrentAmmo; }

protected:
  // ========== INTERNAL ==========

  /** Reads balance stats from DataTable row. Called in BeginPlay before ammo init. */
  void ApplyWeaponDataRow();

  /** Fires a single shot (hitscan trace + damage) — virtual for melee override */
  virtual void FireShot();

  /** Performs a single hitscan line trace */
  void HitscanTrace(const FVector &TraceStart, const FVector &TraceDirection);

  /** Applies recoil to the owning controller */
  void ApplyRecoil();

  /** Recovers recoil each frame */
  void RecoverRecoil(float DeltaTime);

  /** Updates spread bloom each frame */
  void UpdateSpread(float DeltaTime);

  /** Finishes reload */
  void FinishReload();

  /** Sets the weapon state and broadcasts delegate */
  void SetWeaponState(EWeaponState NewState);

  /** Calculates damage with falloff */
  float CalculateDamage(float Distance) const;

  /** Returns the fire interval in seconds */
  float GetFireInterval() const;

  /** ADS blend alpha (0-1) */
  float ADSAlpha = 0.f;

  // ========== BLUEPRINT HOOKS ==========

  /** Called on each shot for BP to spawn VFX, play sounds, etc. */
  UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events",
            meta = (DisplayName = "On Fire"))
  void BP_OnFire(const FVector &MuzzleLocation,
                 const FRotator &MuzzleRotation);

  /** Called on hit for BP to spawn impact effects */
  UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events",
            meta = (DisplayName = "On Hit"))
  void BP_OnHit(const FHitResult &HitResult, float DamageDealt);

  /** Called when reload starts */
  UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events",
            meta = (DisplayName = "On Reload Start"))
  void BP_OnReloadStart();

  /** Called when reload finishes */
  UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events",
            meta = (DisplayName = "On Reload Finish"))
  void BP_OnReloadFinish();
};
