// Copyright RevenantOps. All Rights Reserved.

#include "WeaponBase.h"
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

  CurrentAmmo = MagazineSize;
  CurrentReserveAmmo = MaxReserveAmmo;
  CurrentSpread = BaseSpread;
  CurrentState = EWeaponState::Idle;
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
    if (CurrentAmmo <= 0 && CanReload()) {
      StartReload();
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

  // Play reload montage on character
  if (ReloadMontage && OwnerPawn) {
    if (ACharacter *Character = Cast<ACharacter>(OwnerPawn)) {
      if (UAnimInstance *AnimInstance =
              Character->GetMesh()->GetAnimInstance()) {
        AnimInstance->Montage_Play(ReloadMontage);
      }
    }
  }

  // Play reload sound
  if (ReloadSound) {
    UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
  }

  // Notify Blueprint
  BP_OnReloadStart();

  // Timer to finish reload
  GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this,
                                         &AWeaponBase::FinishReload,
                                         ReloadTime, false);
}

void AWeaponBase::StartADS() { bIsADS = true; }

void AWeaponBase::StopADS() { bIsADS = false; }

bool AWeaponBase::CanFire() const {
  return CurrentState == EWeaponState::Idle && CurrentAmmo > 0;
}

bool AWeaponBase::CanReload() const {
  return CurrentState == EWeaponState::Idle && CurrentAmmo < MagazineSize &&
         CurrentReserveAmmo > 0;
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

  // Get aim direction from camera (center of screen)
  FVector TraceStart;
  FRotator AimRotation;
  if (APlayerController *PC = Cast<APlayerController>(OwnerController)) {
    PC->GetPlayerViewPoint(TraceStart, AimRotation);
  } else {
    TraceStart = MuzzleLocation;
    AimRotation = MuzzleRotation;
  }

  // Play fire sound
  if (FireSound) {
    UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation);
  }

  // Spawn muzzle flash VFX
  if (MuzzleFlashVFX) {
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        this, MuzzleFlashVFX, MuzzleLocation, MuzzleRotation);
  }

  // Fire pellets (1 for normal weapons, multiple for shotguns)
  for (int32 i = 0; i < PelletsPerShot; ++i) {
    // Apply spread
    const float SpreadAngle = GetCurrentSpread();
    const float SpreadRad = FMath::DegreesToRadians(SpreadAngle);
    const FVector SpreadDirection =
        FMath::VRandCone(AimRotation.Vector(), SpreadRad);

    HitscanTrace(TraceStart, SpreadDirection);
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

    // Spawn impact VFX
    if (HitResult.GetActor() && HitResult.GetActor()->ActorHasTag(FName("Enemy"))) {
      if (BloodImpactVFX) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, BloodImpactVFX, HitResult.ImpactPoint,
            HitResult.ImpactNormal.Rotation());
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
  const int32 AmmoToLoad = FMath::Min(AmmoNeeded, CurrentReserveAmmo);

  CurrentAmmo += AmmoToLoad;
  CurrentReserveAmmo -= AmmoToLoad;

  SetWeaponState(EWeaponState::Idle);

  // Notify Blueprint
  BP_OnReloadFinish();

  // Broadcast ammo change
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
