// Copyright RevenantOps. All Rights Reserved.

#include "WeaponBase.h"
#include "WeaponTableRow.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "RevenantOpsPlayerController.h"
#include "RevenantOpsCharacter.h"
#include "UI/RevenantOpsHUD.h"
#include "CameraShakes.h"
#include "DrawDebugHelpers.h"

AWeaponBase::AWeaponBase() {
  PrimaryActorTick.bCanEverTick = true;

  // Create weapon skeletal mesh (root)
  WeaponMesh =
      CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
  RootComponent = WeaponMesh;
  WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // Placeholder static mesh — visible jusqu'à ce qu'un vrai SKM soit assigné
  WeaponMeshSM =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshSM"));
  WeaponMeshSM->SetupAttachment(RootComponent);
  WeaponMeshSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  WeaponMeshSM->SetRelativeScale3D(FVector(0.25f, 0.10f, 0.08f));
  WeaponMeshSM->SetRelativeLocation(FVector(10.f, 0.f, -3.f));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
      TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
  if (CubeMesh.Succeeded()) {
    WeaponMeshSM->SetStaticMesh(CubeMesh.Object);
  }
}

void AWeaponBase::BeginPlay() {
  Super::BeginPlay();

  // Apply DataTable stats BEFORE ammo initialization (per D-09)
  ApplyWeaponDataRow();

  CurrentAmmo = MagazineSize;
  CurrentReserveAmmo = MaxReserveAmmo;
  CurrentSpread = BaseSpread;
  CurrentState = EWeaponState::Idle;
}

void AWeaponBase::ApplyWeaponDataRow()
{
    if (WeaponDataRow.IsNull())
    {
        // No DT assigned — keep constructor defaults (per D-13)
        return;
    }

    static const FString ContextString(TEXT("AWeaponBase::ApplyWeaponDataRow"));
    const FWeaponTableRow* Row = WeaponDataRow.GetRow<FWeaponTableRow>(ContextString);

    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponBase [%s]: DataTable row '%s' not found. Using constructor defaults."),
            *GetName(), *WeaponDataRow.RowName.ToString());
        return;
    }

    // Balance
    BaseDamage   = Row->Damage;
    FireRate     = Row->FireRate;
    MagazineSize = Row->MaxAmmo;
    MaxRange     = Row->Range;

    // Crosshair — source unique : DT_WeaponStats
    CrosshairStyle     = Row->CrosshairStyle;
    ScopeFOVMultiplier = Row->ScopeFOVMultiplier;

    // Scope texture : résolution synchrone (asset déjà cooked/chargé en éditeur)
    if (!Row->ScopeOverlayTexture.IsNull())
    {
        ScopeOverlayTexture = Row->ScopeOverlayTexture.LoadSynchronous();
    }
    else
    {
        ScopeOverlayTexture = nullptr;
    }
}

void AWeaponBase::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  RecoverRecoil(DeltaTime);
  UpdateSpread(DeltaTime);

  // ADS alpha interpolation
  const float TargetAlpha = bIsADS ? 1.f : 0.f;
  ADSAlpha = FMath::FInterpTo(ADSAlpha, TargetAlpha, DeltaTime, ADSInterpSpeed);

  // Auto-fire for full-auto and burst modes
  if (bWantsToFire && CanFire()) {
    const float TimeSinceLastFire =
        GetWorld()->GetTimeSeconds() - LastFireTime;
    if (TimeSinceLastFire >= GetFireInterval()) {
      FireShot();
    }
  }
}

// =============================================================================
// PUBLIC API
// =============================================================================

void AWeaponBase::SetOwnerPawn(APawn *NewOwner) {
  OwnerPawn = NewOwner;
  if (NewOwner) {
    OwnerController = NewOwner->GetController();
    SetOwner(NewOwner);
  } else {
    OwnerController = nullptr;
    SetOwner(nullptr);
  }
}

void AWeaponBase::StartFire() {
  if (!CanFire()) {
    // Auto-reload when trying to fire with empty magazine
    if (CurrentAmmo <= 0) {
      if (EmptySound) {
        UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
      }
      if (CanReload()) {
        StartReload();
      }
    }
    return;
  }

  bWantsToFire = true;

  // Fire immediately
  const float TimeSinceLastFire =
      GetWorld()->GetTimeSeconds() - LastFireTime;
  if (TimeSinceLastFire >= GetFireInterval()) {
    if (FireMode == EWeaponFireMode::Burst) {
      BurstShotsRemaining = BurstCount;
    }
    FireShot();
  }
}

void AWeaponBase::StopFire() {
  bWantsToFire = false;
  BurstShotsRemaining = 0;
}

void AWeaponBase::StartReload() {
  if (!CanReload()) {
    return;
  }

  SetWeaponState(EWeaponState::Reloading);
  StopFire();
  ReloadStartTime = GetWorld()->GetTimeSeconds();

  // Play reload montage on character
  UE_LOG(LogTemp, Warning, TEXT("StartReload: ReloadMontage=%s OwnerPawn=%s"),
    ReloadMontage ? *ReloadMontage->GetName() : TEXT("NULL"),
    OwnerPawn ? *OwnerPawn->GetName() : TEXT("NULL"));
  if (ReloadMontage && OwnerPawn) {
    if (ACharacter *Character = Cast<ACharacter>(OwnerPawn)) {
      if (UAnimInstance *AnimInstance =
              Character->GetMesh()->GetAnimInstance()) {
        float Result = AnimInstance->Montage_Play(ReloadMontage);
        UE_LOG(LogTemp, Warning, TEXT("Montage_Play result: %f"), Result);
      } else {
        UE_LOG(LogTemp, Warning, TEXT("StartReload: AnimInstance is NULL"));
      }
    } else {
      UE_LOG(LogTemp, Warning, TEXT("StartReload: Cast to ACharacter failed"));
    }
  }

  // Play reload sound
  if (ReloadSound) {
    UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
  }

  // Notify Blueprint
  BP_OnReloadStart();

  // Timer to finish reload — réduit par ReloadSpeedMultiplier du personnage
  float ActualReloadTime = ReloadTime;
  if (ARevenantOpsCharacter* Merc = Cast<ARevenantOpsCharacter>(OwnerPawn))
    ActualReloadTime = FMath::Max(0.3f, ReloadTime / Merc->ReloadSpeedMultiplier);
  GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this,
                                         &AWeaponBase::FinishReload,
                                         ActualReloadTime, false);
}

void AWeaponBase::StartADS() { bIsADS = true; }

void AWeaponBase::StopADS() { bIsADS = false; }

int32 AWeaponBase::GetCurrentReserveAmmo() const
{
  if (ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(OwnerPawn))
    return Char->GetInventoryAmmo(WeaponAmmoType);
  return CurrentReserveAmmo;
}

bool AWeaponBase::CanFire() const {
  if (CurrentState != EWeaponState::Idle || CurrentAmmo <= 0) return false;
  // RE5 style — tir uniquement en ADS
  if (ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(OwnerPawn))
  {
    if (!Char->IsAiming()) return false;
  }
  return true;
}

bool AWeaponBase::CanReload() const {
  if (CurrentState != EWeaponState::Idle || CurrentAmmo >= MagazineSize)
    return false;

  // RE5 : les munitions de réserve sont dans l'inventaire du personnage
  if (ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(OwnerPawn))
    return Char->GetInventoryAmmo(WeaponAmmoType) > 0;

  // Fallback legacy (armes sans owner char)
  return CurrentReserveAmmo > 0;
}

float AWeaponBase::GetCurrentSpread() const {
  return CurrentSpread * (bIsADS ? ADSSpreadMultiplier : 1.f);
}

float AWeaponBase::GetFireInterval() const {
  return 60.f / FMath::Max(FireRate, 1.f);
}

// =============================================================================
// FIRING
// =============================================================================

void AWeaponBase::FireShot() {
  if (!OwnerPawn || OwnerPawn->IsPendingKillPending()) {
    StopFire();
    return;
  }

  if (CurrentAmmo <= 0) {
    // Auto-reload
    if (CanReload()) {
      StartReload();
    }
    bWantsToFire = false;
    return;
  }

  // Consume ammo
  --CurrentAmmo;
  LastFireTime = GetWorld()->GetTimeSeconds();
  SetWeaponState(EWeaponState::Firing);

  // Get muzzle transform
  FVector MuzzleLocation = FVector::ZeroVector;
  FRotator MuzzleRotation = FRotator::ZeroRotator;
  if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName)) {
    MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
    MuzzleRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);
  } else if (OwnerPawn) {
    // Fallback: use camera
    if (APlayerController *PC =
            Cast<APlayerController>(OwnerController)) {
      PC->GetPlayerViewPoint(MuzzleLocation, MuzzleRotation);
    }
  }

  // ── Tir standard TPS (RE5 / Gears of War) ───────────────────────────────
  //
  // Les DÉGÂTS viennent des yeux (GetPawnViewLocation) — toujours dans la
  // capsule, jamais dans un mur. Ce que le crosshair vise = ce qui est touché.
  //
  // Le MUZZLE sert uniquement aux VFX (flash, son). Il n'est pas l'origine
  // du rayon de dégâts → plus de parallaxe OTS, crosshair = impact garanti.

  FRotator CamRot;
  FVector  EyeLocation   = MuzzleLocation; // fallback si pas de PC
  FVector  AimEnd        = MuzzleLocation + MuzzleRotation.Vector() * MaxRange;

  if (APlayerController* PC = Cast<APlayerController>(OwnerController))
  {
    // CamPos = position exacte de la caméra = origine du rayon crosshair.
    // GetPawnViewLocation() est décalé bas-gauche vs caméra OTS → ne pas utiliser.
    FVector CamPos;
    PC->GetPlayerViewPoint(CamPos, CamRot);
    EyeLocation = CamPos;
    AimEnd      = CamPos + CamRot.Vector() * MaxRange;
  }

  // VFX uniquement au muzzle — avant la trace de dégâts
  if (FireSound)
    UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation);

  if (MuzzleFlashVFX && WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleFlashVFX, MuzzleLocation, MuzzleRotation);

  // Spread appliqué sur la direction de visée (depuis les yeux)
  const float   SpreadAngle = GetCurrentSpread();
  const float   SpreadRad   = FMath::DegreesToRadians(SpreadAngle);
  const FVector AimDir      = (AimEnd - EyeLocation).GetSafeNormal();

  // Tir des projectiles — dégâts depuis les yeux, crosshair = impact
  for (int32 i = 0; i < PelletsPerShot; ++i)
  {
    const FVector SpreadDir = FMath::VRandCone(AimDir, SpreadRad);
    HitscanTrace(EyeLocation, SpreadDir);
  }

  // Apply recoil
  ApplyRecoil();

  // Increase spread bloom
  CurrentSpread = FMath::Min(CurrentSpread + SpreadPerShot, MaxSpread);

  // Play character fire montage
  if (CharacterFireMontage && OwnerPawn) {
    if (ACharacter *Character = Cast<ACharacter>(OwnerPawn)) {
      if (UAnimInstance *AnimInstance =
              Character->GetMesh()->GetAnimInstance()) {
        AnimInstance->Montage_Play(CharacterFireMontage);
      }
    }
  }

  // Notify Blueprint for VFX/SFX
  BP_OnFire(MuzzleLocation, MuzzleRotation);

  // Broadcast ammo change
  OnAmmoChanged.Broadcast(CurrentAmmo, MagazineSize);

  // Handle fire mode logic
  if (FireMode == EWeaponFireMode::SemiAuto) {
    bWantsToFire = false;
  } else if (FireMode == EWeaponFireMode::Burst) {
    --BurstShotsRemaining;
    if (BurstShotsRemaining <= 0) {
      bWantsToFire = false;
    }
  }

  // Return to idle after the fire frame
  SetWeaponState(EWeaponState::Idle);

  // Camera shake on fire
  if (APlayerController *PC = Cast<APlayerController>(OwnerController)) {
    PC->ClientStartCameraShake(UCS_WeaponFire::StaticClass(), 0.5f);
  }

  // Auto-reload if magazine is now empty
  if (CurrentAmmo <= 0 && CanReload()) {
    StartReload();
  }
}

void AWeaponBase::HitscanTrace(const FVector &TraceStart,
                                const FVector &TraceDirection) {
  const FVector TraceEnd = TraceStart + TraceDirection * MaxRange;

  FHitResult HitResult;
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(this);
  if (OwnerPawn) {
    QueryParams.AddIgnoredActor(OwnerPawn);
  }
  QueryParams.bReturnPhysicalMaterial = true;

  const bool bHit = GetWorld()->LineTraceSingleByChannel(
      HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);


  if (bHit) {
    const float HitDistance = FVector::Dist(TraceStart, HitResult.ImpactPoint);
    float Damage = CalculateDamage(HitDistance);

    UE_LOG(LogTemp, Warning, TEXT("[HitscanTrace] HIT: %s (Bone: %s) Dist=%.0f Damage=%.1f"),
        *HitResult.GetActor()->GetName(), *HitResult.BoneName.ToString(),
        HitDistance, Damage);

    // Check for headshot (bone name convention)
    if (HitResult.BoneName == FName("head") ||
        HitResult.BoneName == FName("Head")) {
      Damage *= HeadshotMultiplier;
    }

    // Apply damage to hit actor
    if (HitResult.GetActor()) {
      FPointDamageEvent DamageEvent(Damage, HitResult, TraceDirection,
                                    nullptr);
      HitResult.GetActor()->TakeDamage(Damage, DamageEvent, OwnerController,
                                        this);
    }

    // Spawn impact VFX + hit marker uniquement sur les ennemis
    if (HitResult.GetActor() && HitResult.GetActor()->ActorHasTag(FName("Enemy"))) {
      if (BloodImpactVFX) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, BloodImpactVFX, HitResult.ImpactPoint,
            HitResult.ImpactNormal.Rotation());
      }
      // Viseur clignote rouge uniquement sur hit ennemi
      if (ARevenantOpsPlayerController* ROPC = Cast<ARevenantOpsPlayerController>(OwnerController)) {
        if (URevenantOpsHUD* HUD = ROPC->GetHUDWidget()) {
          HUD->ShowHitMarker();
        }
      }
    } else if (ImpactVFX) {
      UNiagaraFunctionLibrary::SpawnSystemAtLocation(
          this, ImpactVFX, HitResult.ImpactPoint,
          HitResult.ImpactNormal.Rotation());
    }

    // Notify Blueprint for impact effects
    BP_OnHit(HitResult, Damage);
  }
}

float AWeaponBase::CalculateDamage(float Distance) const {
  if (Distance <= DamageFalloffStart) {
    return BaseDamage;
  }

  if (Distance >= DamageFalloffEnd) {
    return BaseDamage * MinDamageMultiplier;
  }

  // Linear falloff between start and end
  const float FalloffRange = DamageFalloffEnd - DamageFalloffStart;
  const float FalloffAlpha = (Distance - DamageFalloffStart) / FalloffRange;
  const float DamageMultiplier =
      FMath::Lerp(1.0f, MinDamageMultiplier, FalloffAlpha);

  return BaseDamage * DamageMultiplier;
}

// =============================================================================
// RECOIL
// =============================================================================

void AWeaponBase::ApplyRecoil() {
  if (!OwnerController) {
    return;
  }

  const float RecoilMultiplier = bIsADS ? ADSRecoilMultiplier : 1.f;

  // Vertical recoil (always kicks up)
  const float Pitch = -VerticalRecoil * RecoilMultiplier;

  // Horizontal recoil (random left/right)
  const float Yaw = FMath::FRandRange(-HorizontalRecoilRange,
                                       HorizontalRecoilRange) *
                     RecoilMultiplier;

  // Apply to controller (AddPitchInput/AddYawInput are on APlayerController)
  if (APlayerController *PC = Cast<APlayerController>(OwnerController)) {
    PC->AddPitchInput(Pitch);
    PC->AddYawInput(Yaw);
  }

  // Track accumulated recoil for recovery
  AccumulatedRecoil.X += Pitch;
  AccumulatedRecoil.Y += Yaw;
}

void AWeaponBase::RecoverRecoil(float DeltaTime) {
  if (AccumulatedRecoil.IsNearlyZero(0.01f) || !OwnerController) {
    AccumulatedRecoil = FVector2D::ZeroVector;
    return;
  }

  // Only recover when not firing
  if (bWantsToFire) {
    return;
  }

  APlayerController *PC = Cast<APlayerController>(OwnerController);
  if (!PC) {
    return;
  }

  // Interpolate accumulated recoil towards zero
  const float NewPitch =
      FMath::FInterpTo(AccumulatedRecoil.X, 0.f, DeltaTime, RecoilRecoverySpeed);
  const float PitchRecovery = AccumulatedRecoil.X - NewPitch;
  PC->AddPitchInput(-PitchRecovery);
  AccumulatedRecoil.X = NewPitch;

  const float NewYaw =
      FMath::FInterpTo(AccumulatedRecoil.Y, 0.f, DeltaTime, RecoilRecoverySpeed);
  const float YawRecovery = AccumulatedRecoil.Y - NewYaw;
  PC->AddYawInput(-YawRecovery);
  AccumulatedRecoil.Y = NewYaw;
}

// =============================================================================
// SPREAD
// =============================================================================

void AWeaponBase::UpdateSpread(float DeltaTime) {
  // Recover spread towards base when not firing
  if (!bWantsToFire) {
    CurrentSpread = FMath::FInterpTo(CurrentSpread, BaseSpread, DeltaTime,
                                     SpreadRecoverySpeed);
  }
}

// =============================================================================
// RELOAD
// =============================================================================

void AWeaponBase::FinishReload() {
  const int32 AmmoNeeded = MagazineSize - CurrentAmmo;

  // RE5 : consommer depuis l'inventaire du personnage
  int32 AmmoLoaded = 0;
  if (ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(OwnerPawn))
  {
    AmmoLoaded = Char->ConsumeInventoryAmmo(WeaponAmmoType, AmmoNeeded);
  }
  else
  {
    // Fallback legacy
    AmmoLoaded = FMath::Min(AmmoNeeded, CurrentReserveAmmo);
    CurrentReserveAmmo -= AmmoLoaded;
  }

  CurrentAmmo += AmmoLoaded;

  SetWeaponState(EWeaponState::Idle);

  // Notify Blueprint
  BP_OnReloadFinish();

  // Broadcast ammo change — le HUD lira la réserve via GetCurrentReserveAmmo()
  OnAmmoChanged.Broadcast(CurrentAmmo, MagazineSize);
}

// =============================================================================
// STATE
// =============================================================================

void AWeaponBase::SetWeaponState(EWeaponState NewState) {
  if (CurrentState != NewState) {
    CurrentState = NewState;
    OnWeaponStateChanged.Broadcast(NewState);
  }
}

// =============================================================================
// RELOAD PROGRESS
// =============================================================================

float AWeaponBase::GetReloadProgress() const {
  if (CurrentState != EWeaponState::Reloading) {
    return 0.f;
  }
  const float Elapsed = GetWorld()->GetTimeSeconds() - ReloadStartTime;
  return FMath::Clamp(Elapsed / ReloadTime, 0.f, 1.f);
}
