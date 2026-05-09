// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevenantOpsCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "RevenantOps.h"
#include "TimerManager.h"
#include "WeaponBase.h"
#include "HealthComponent.h"
#include "CameraShakes.h"
#include "RevenantOpsPlayerController.h"
#include "UI/RevenantOpsHUD.h"
#include "Gameplay/AmmoBonusPickup.h"
#include "Gameplay/HealthPickup.h"
#include "Gameplay/InventoryDropPickup.h"
#include "Gameplay/PickupInterface.h"
#include "Gameplay/WeaponPickup.h"
#include "Kismet/GameplayStatics.h"
#include "AI/EnemyBase.h"

ARevenantOpsCharacter::ARevenantOpsCharacter() {
  PrimaryActorTick.bCanEverTick = true;

  Tags.Add(FName("Player"));

  // Collision capsule
  GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

  // Don't rotate when the controller rotates
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;

  // Character movement defaults
  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
  GetCharacterMovement()->JumpZVelocity = 500.f;
  GetCharacterMovement()->AirControl = 0.35f;
  GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
  GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
  GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
  GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

  // Enable crouching
  GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
  GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchMoveSpeed;
  GetCharacterMovement()->SetCrouchedHalfHeight(58.f);

  // Camera boom (over-the-shoulder RE4/RE5 style)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = CameraHipArmLength;
  CameraBoom->SocketOffset   = CameraHipOffset;
  CameraBoom->bUsePawnControlRotation = true;
  CameraBoom->bDoCollisionTest = true;
  CameraBoom->ProbeSize = 12.f;
  CameraBoom->ProbeChannel = ECC_Camera;
  CameraBoom->bEnableCameraLag = true;
  CameraBoom->CameraLagSpeed = 15.f;
  CameraBoom->bEnableCameraRotationLag = true;
  CameraBoom->CameraRotationLagSpeed = 20.f;

  // Follow camera
  FollowCamera =
      CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom,
                                USpringArmComponent::SocketName);
  FollowCamera->bUsePawnControlRotation = false;

  // Bind dodge montage delegate
  OnDodgeMontageEnded.BindUObject(this,
                                  &ARevenantOpsCharacter::DodgeMontageEnded);
}

void ARevenantOpsCharacter::BeginPlay() {
  Super::BeginPlay();

  // Cache default movement values for slide restore
  DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
  DefaultBrakingDeceleration =
      GetCharacterMovement()->BrakingDecelerationWalking;

  // Initialize stamina
  CurrentStamina = MaxStamina;

  // Set initial speed
  TargetSpeed = WalkSpeed;
  GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

  // Set initial FOV
  if (FollowCamera) {
    FollowCamera->SetFieldOfView(DefaultFOV);
  }

  // Cache health component and bind damage feedback
  HealthComp = FindComponentByClass<UHealthComponent>();
  if (HealthComp)
    HealthComp->OnHealthChanged.AddDynamic(this, &ARevenantOpsCharacter::OnDamageReceived);

  // Appliquer les perks hardcodés + les talents assignés (écran de sélection)
  ApplyTalents();

  // Initialize inventory — 9 empty slots
  Inventory.SetNum(9);
  for (FInventoryItem& Slot : Inventory) { Slot = FInventoryItem(); }

  // Spawn weapons from loadout
  SpawnDefaultWeapons();
}

// =============================================================================
// TALENTS
// =============================================================================

void ARevenantOpsCharacter::ApplyTalents()
{
    for (UTalentDefinition* Talent : AssignedTalents)
    {
        if (!Talent) continue;

        // Reload speed (multiplicatif)
        if (Talent->ReloadSpeedBonus > 0.f)
            ReloadSpeedMultiplier += Talent->ReloadSpeedBonus;

        // Damage resistance (additif, clampé à 0.75)
        if (Talent->DamageResistanceBonus > 0.f)
            DamageResistance = FMath::Min(DamageResistance + Talent->DamageResistanceBonus, 0.75f);

        // Ammo capacity (multiplicatif)
        if (Talent->AmmoCapacityBonus > 0.f)
            AmmoCapacityMultiplier += Talent->AmmoCapacityBonus;

        // Move speed (additif)
        if (Talent->MoveSpeedBonus > 0.f)
            MoveSpeedBonus += Talent->MoveSpeedBonus;

        // Max health bonus — appliqué directement sur HealthComp si disponible
        if (Talent->MaxHealthBonus > 0.f)
        {
            if (!HealthComp) HealthComp = FindComponentByClass<UHealthComponent>();
            if (HealthComp)
            {
                HealthComp->SetMaxHealth(HealthComp->GetMaxHealth() * (1.f + Talent->MaxHealthBonus));
                HealthComp->ResetHealth(); // CurrentHealth = nouveau max
            }
        }

        // Stamina bonus
        if (Talent->StaminaBonus > 0.f)
        {
            MaxStamina = FMath::Min(MaxStamina * (1.f + Talent->StaminaBonus), 200.f);
            CurrentStamina = MaxStamina;
        }
    }

    // Re-appliquer les stats calculées (résistance, vitesse) maintenant que les talents sont cumulés
    if (HealthComp)
        HealthComp->DamageMultiplier = FMath::Clamp(1.f - DamageResistance, 0.25f, 1.f);

    if (MoveSpeedBonus > 0.f)
    {
        // MoveSpeedBonus accumulé = somme des % talents (ex: 0.2 + 0.1 = 0.3 = +30%)
        WalkSpeed   *= (1.f + MoveSpeedBonus);
        SprintSpeed *= (1.f + MoveSpeedBonus);
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        MoveSpeedBonus = 0.f; // reset pour éviter double-application si rappelé
    }
}

// =============================================================================
// TICK
// =============================================================================

void ARevenantOpsCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  UpdateMovementSpeed(DeltaTime);
  UpdateStamina(DeltaTime);
  UpdateCameraFOV(DeltaTime);
  UpdateAnimationValues();
  UpdateAimRotation(DeltaTime);

  // Auto-end slide if speed drops too low
  if (bIsSliding) {
    const float CurrentGroundSpeed =
        GetCharacterMovement()->Velocity.Size2D();
    if (CurrentGroundSpeed < SlideMinSpeed) {
      EndSlide();
    }
  }
}

// =============================================================================
// INPUT SETUP
// =============================================================================

void ARevenantOpsCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  if (UEnhancedInputComponent *EIC =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

    // Jumping
    EIC->BindAction(JumpAction, ETriggerEvent::Started, this,
                    &ACharacter::Jump);
    EIC->BindAction(JumpAction, ETriggerEvent::Completed, this,
                    &ACharacter::StopJumping);

    // Moving
    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this,
                    &ARevenantOpsCharacter::Move);
    EIC->BindAction(MouseLookAction, ETriggerEvent::Triggered, this,
                    &ARevenantOpsCharacter::Look);

    // Looking
    EIC->BindAction(LookAction, ETriggerEvent::Triggered, this,
                    &ARevenantOpsCharacter::Look);

    // Sprinting (hold)
    EIC->BindAction(SprintAction, ETriggerEvent::Triggered, this,
                    &ARevenantOpsCharacter::StartSprint);
    EIC->BindAction(SprintAction, ETriggerEvent::Completed, this,
                    &ARevenantOpsCharacter::StopSprint);

    // Crouching (toggle)
    if (CrouchAction) {
      EIC->BindAction(CrouchAction, ETriggerEvent::Started, this,
                      &ARevenantOpsCharacter::CrouchPressed);
    }

    // Dodge/Roll
    if (DodgeAction) {
      EIC->BindAction(DodgeAction, ETriggerEvent::Started, this,
                      &ARevenantOpsCharacter::DodgePressed);
    }

    // Fire (hold for auto)
    if (FireAction) {
      EIC->BindAction(FireAction, ETriggerEvent::Started, this,
                      &ARevenantOpsCharacter::FirePressed);
      EIC->BindAction(FireAction, ETriggerEvent::Completed, this,
                      &ARevenantOpsCharacter::FireReleased);
    }

    // ADS / Aim (hold)
    if (AimAction) {
      EIC->BindAction(AimAction, ETriggerEvent::Started, this,
                      &ARevenantOpsCharacter::AimPressed);
      EIC->BindAction(AimAction, ETriggerEvent::Completed, this,
                      &ARevenantOpsCharacter::AimReleased);
    }

    // Reload
    if (ReloadAction) {
      EIC->BindAction(ReloadAction, ETriggerEvent::Started, this,
                      &ARevenantOpsCharacter::ReloadPressed);
    }

    // Switch Weapon (scroll wheel or key)
    if (SwitchWeaponAction) {
      EIC->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this,
                      &ARevenantOpsCharacter::SwitchWeaponPressed);
    }

    if (InteractAction) {
      EIC->BindAction(InteractAction, ETriggerEvent::Triggered, this,
                      &ARevenantOpsCharacter::InteractPressed);
    }

    // Melee attack
    if (MeleeAction) {
      EIC->BindAction(MeleeAction, ETriggerEvent::Started, this,
                      &ARevenantOpsCharacter::MeleeAttackPressed);
    }

  } else {
    UE_LOG(
        LogRevenantOps, Error,
        TEXT("'%s' Failed to find an Enhanced Input component! This template "
             "is built to use the Enhanced Input system. If you intend to use "
             "the legacy system, then you will need to update this C++ file."),
        *GetNameSafe(this));
  }
}

// =============================================================================
// MOVEMENT INPUT
// =============================================================================

void ARevenantOpsCharacter::Move(const FInputActionValue &Value) {
  FVector2D MovementVector = Value.Get<FVector2D>();
  DoMove(MovementVector.X, MovementVector.Y);
}

void ARevenantOpsCharacter::Look(const FInputActionValue &Value) {
  FVector2D LookAxisVector = Value.Get<FVector2D>();
  DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ARevenantOpsCharacter::DoMove(float Right, float Forward) {
  if (GetController() == nullptr) {
    return;
  }

  // No manual movement while sliding
  if (bIsSliding) {
    return;
  }

  const FRotator Rotation = GetController()->GetControlRotation();
  const FRotator YawRotation(0, Rotation.Yaw, 0);

  const FVector ForwardDirection =
      FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
  const FVector RightDirection =
      FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

  AddMovementInput(ForwardDirection, Forward);
  AddMovementInput(RightDirection, Right);
}

void ARevenantOpsCharacter::DoLook(float Yaw, float Pitch) {
  if (GetController() != nullptr) {
    AddControllerYawInput(Yaw);
    AddControllerPitchInput(Pitch);
  }
}

void ARevenantOpsCharacter::DoJumpStart() {
  if (bIsSliding) {
    EndSlide();
  }
  Jump();
}

void ARevenantOpsCharacter::DoJumpEnd() { StopJumping(); }

// =============================================================================
// SPRINT
// =============================================================================

void ARevenantOpsCharacter::StartSprint() {
  bWantsToSprint = true;

  const bool bWeaponReloading = CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Reloading;
  if (bStaminaDepleted || bIsCrouched || bIsSliding || bIsAiming || bWeaponReloading) {
    return;
  }

  bIsSprinting = true;
  TargetSpeed = SprintSpeed;

  // Montage optionnel de départ sprint (upper body only — slot "UpperBody")
  if (SprintStartMontage)
  {
    PlayAnimMontage(SprintStartMontage);
  }
}

void ARevenantOpsCharacter::StopSprint() {
  bWantsToSprint = false;
  bIsSprinting = false;

  if (!bIsCrouched && !bIsSliding) {
    TargetSpeed = WalkSpeed;
  }
}

// =============================================================================
// CROUCH & SLIDE
// =============================================================================

void ARevenantOpsCharacter::CrouchPressed() {
  if (bIsDodging) {
    return;
  }

  if (bIsSliding) {
    EndSlide();
    return;
  }

  if (bIsCrouched) {
    UnCrouch();
    if (bWantsToSprint && !bStaminaDepleted) {
      bIsSprinting = true;
      TargetSpeed = SprintSpeed;
    } else {
      TargetSpeed = WalkSpeed;
    }
  } else {
    // Sprint + Crouch = Slide
    if (bIsSprinting &&
        GetCharacterMovement()->Velocity.Size2D() > WalkSpeed * 0.8f) {
      StartSlide();
    } else {
      Crouch();
      bIsSprinting = false;
      TargetSpeed = CrouchMoveSpeed;
    }
  }
}

// =============================================================================
// SLIDE
// =============================================================================

void ARevenantOpsCharacter::StartSlide() {
  if (!ConsumeStamina(SlideCost)) {
    Crouch();
    bIsSprinting = false;
    TargetSpeed = CrouchMoveSpeed;
    return;
  }

  bIsSliding = true;
  bIsSprinting = false;

  Crouch();

  GetCharacterMovement()->GroundFriction = SlideGroundFriction;
  GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration;
  GetCharacterMovement()->MaxWalkSpeedCrouched = SlideBoostSpeed;

  const FVector SlideDirection = GetActorForwardVector();
  LaunchCharacter(SlideDirection * SlideBoostSpeed, true, false);

  GetWorld()->GetTimerManager().SetTimer(
      SlideTimerHandle, this, &ARevenantOpsCharacter::EndSlide,
      SlideMaxDuration, false);
}

void ARevenantOpsCharacter::EndSlide() {
  if (!bIsSliding) {
    return;
  }

  bIsSliding = false;
  GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandle);

  GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
  GetCharacterMovement()->BrakingDecelerationWalking =
      DefaultBrakingDeceleration;
  GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchMoveSpeed;

  UnCrouch();

  if (bWantsToSprint && !bStaminaDepleted) {
    bIsSprinting = true;
    TargetSpeed = SprintSpeed;
  } else {
    TargetSpeed = WalkSpeed;
  }
}

// =============================================================================
// DODGE / ROLL
// =============================================================================

void ARevenantOpsCharacter::DodgePressed() { StartDodge(); }

void ARevenantOpsCharacter::StartDodge() {
  const float CurrentTime = GetWorld()->GetTimeSeconds();
  if (CurrentTime - LastDodgeTime < DodgeCooldown) {
    return;
  }

  if (bIsDodging || bIsSliding) {
    return;
  }

  if (!ConsumeStamina(DodgeCost)) {
    return;
  }

  bIsDodging = true;
  LastDodgeTime = CurrentTime;

  FVector DodgeDirection;
  const FVector InputVector = GetCharacterMovement()->GetLastInputVector();
  if (InputVector.SizeSquared() > 0.1f) {
    DodgeDirection = InputVector.GetSafeNormal();
  } else {
    DodgeDirection = GetActorForwardVector();
  }

  LaunchCharacter(DodgeDirection * DodgeLaunchForce, true, false);

  if (DodgeMontage) {
    if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
      const float MontageLength = AnimInstance->Montage_Play(DodgeMontage);
      if (MontageLength > 0.f) {
        AnimInstance->Montage_SetEndDelegate(OnDodgeMontageEnded, DodgeMontage);
      } else {
        bIsDodging = false;
      }
    }
  } else {
    GetWorld()->GetTimerManager().SetTimer(
        DodgeEndTimerHandle,
        [this]() { bIsDodging = false; }, 0.5f, false);
  }
}

void ARevenantOpsCharacter::DodgeMontageEnded(UAnimMontage *Montage,
                                               bool bInterrupted) {
  bIsDodging = false;
}

// =============================================================================
// STAMINA
// =============================================================================

bool ARevenantOpsCharacter::ConsumeStamina(float Amount) {
  if (CurrentStamina < Amount) {
    return false;
  }

  CurrentStamina -= Amount;
  LastStaminaDrainTime = GetWorld()->GetTimeSeconds();

  if (CurrentStamina <= 0.f) {
    CurrentStamina = 0.f;
    bStaminaDepleted = true;

    if (bIsSprinting) {
      bIsSprinting = false;
      TargetSpeed = bIsCrouched ? CrouchMoveSpeed : WalkSpeed;
    }
  }

  return true;
}

void ARevenantOpsCharacter::UpdateStamina(float DeltaTime) {
  const float CurrentTime = GetWorld()->GetTimeSeconds();

  if (bIsSprinting && GetCharacterMovement()->IsMovingOnGround() &&
      GetCharacterMovement()->Velocity.SizeSquared() > 0.f) {
    CurrentStamina -= SprintStaminaDrain * DeltaTime;
    LastStaminaDrainTime = CurrentTime;

    if (CurrentStamina <= 0.f) {
      CurrentStamina = 0.f;
      bStaminaDepleted = true;
      bIsSprinting = false;
      TargetSpeed = WalkSpeed;
    }
  } else if (CurrentTime - LastStaminaDrainTime >= StaminaRegenDelay) {
    CurrentStamina =
        FMath::Min(CurrentStamina + StaminaRegenRate * DeltaTime, MaxStamina);

    if (bStaminaDepleted && CurrentStamina >= StaminaRecoveryThreshold) {
      bStaminaDepleted = false;

      const bool bWeaponReloading = CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Reloading;
      if (bWantsToSprint && !bIsCrouched && !bIsSliding && !bWeaponReloading) {
        bIsSprinting = true;
        TargetSpeed = SprintSpeed;
      }
    }
  }
}

float ARevenantOpsCharacter::GetStaminaPercent() const {
  return (MaxStamina > 0.f) ? CurrentStamina / MaxStamina : 0.f;
}

// =============================================================================
// SPEED & CAMERA INTERPOLATION
// =============================================================================

void ARevenantOpsCharacter::UpdateMovementSpeed(float DeltaTime) {
  if (bIsSliding) {
    return;
  }

  // ADS slows movement
  float EffectiveTarget = TargetSpeed;
  if (bIsAiming && CurrentWeapon) {
    EffectiveTarget *= 0.6f;
  }

  // Reload : plafonner la vitesse a WalkSpeed sans casser les flags d'animation
  if (CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Reloading) {
    EffectiveTarget = FMath::Min(EffectiveTarget, WalkSpeed);
  }

  const float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
  const float NewSpeed =
      FMath::FInterpTo(CurrentSpeed, EffectiveTarget, DeltaTime, SpeedInterpRate);
  GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ARevenantOpsCharacter::UpdateCameraFOV(float DeltaTime) {
  if (!FollowCamera || !CameraBoom) {
    return;
  }

  // Offsets lus depuis les UPROPERTY — modifiables dans le BP sans recompiler
  const FVector HipOffset    = CameraHipOffset;
  const FVector ADSOffset    = CameraADSOffset;
  const float   HipArmLength = CameraHipArmLength;
  const float   ADSArmLength = CameraADSArmLength;

  float TargetFOV = DefaultFOV;
  FVector TargetOffset = HipOffset;
  float TargetArmLen = HipArmLength;

  if (bIsAiming && CurrentWeapon) {
    // Use weapon's ADSFOV for zoom level
    const float WeaponADSFOV = CurrentWeapon->GetADSFOV();
    const float Alpha = CurrentWeapon->GetADSAlpha();
    TargetFOV = FMath::Lerp(DefaultFOV, WeaponADSFOV, Alpha);
    TargetOffset = FMath::Lerp(HipOffset, ADSOffset, Alpha);
    TargetArmLen = FMath::Lerp(HipArmLength, ADSArmLength, Alpha);
  } else if (bIsSprinting) {
    TargetFOV = SprintFOV;
  }

  // Smooth FOV transition
  const float CurrentFOV = FollowCamera->FieldOfView;
  const float NewFOV =
      FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
  FollowCamera->SetFieldOfView(NewFOV);

  // Smooth boom offset transition
  const FVector CurrentOffset = CameraBoom->SocketOffset;
  const FVector NewOffset =
      FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, FOVInterpSpeed);
  CameraBoom->SocketOffset = NewOffset;

  // Smooth arm length transition
  const float CurrentArmLen = CameraBoom->TargetArmLength;
  const float NewArmLen =
      FMath::FInterpTo(CurrentArmLen, TargetArmLen, DeltaTime, FOVInterpSpeed);
  CameraBoom->TargetArmLength = NewArmLen;
}

void ARevenantOpsCharacter::UpdateAimRotation(float DeltaTime)
{
  const bool bShouldOrientToAim = bIsArmed && !bIsSprinting && !bIsDodging;

  if (bShouldOrientToAim)
  {
    // Le personnage fait face à la direction de visée
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
  }
  else
  {
    // Rotation naturelle vers la direction de déplacement
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
  }
}

// =============================================================================
// MELEE
// =============================================================================

void ARevenantOpsCharacter::MeleeAttackPressed()
{
  const float Now = GetWorld()->GetTimeSeconds();
  if (Now - LastMeleeTime < MeleeCooldown) return;
  if (bIsSprinting || bIsSliding) return;

  LastMeleeTime = Now;

  // Montage animé
  if (MeleeMontage)
    PlayAnimMontage(MeleeMontage);

  // ── Sweep de zone devant le joueur (contrôle de foule) ────────────────────
  // Large sphère centrée sur le joueur — touche plusieurs ennemis à la fois
  const FVector TraceCenter = GetActorLocation() + GetActorForwardVector() * (MeleeRange * 0.5f);
  const float   SphereRadius = MeleeRange * 0.6f;   // zone généreuse

  TArray<FHitResult> Hits;
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  GetWorld()->SweepMultiByChannel(
    Hits, GetActorLocation(), TraceCenter,
    FQuat::Identity,
    ECC_Pawn,
    FCollisionShape::MakeSphere(SphereRadius),
    Params
  );

  // Dédupliquer les acteurs (plusieurs bones du même ennemi peuvent être touchés)
  TSet<AActor*> AlreadyHit;

  for (const FHitResult& Hit : Hits)
  {
    AActor* Target = Hit.GetActor();
    if (!Target || AlreadyHit.Contains(Target)) continue;
    if (!Target->ActorHasTag(FName("Enemy"))) continue;

    AlreadyHit.Add(Target);

    AEnemyBase* Enemy = Cast<AEnemyBase>(Target);

    if (Enemy && Enemy->IsVulnerableToFinisher())
    {
      // ── FINISHER : ennemi touché en zone vitale ───────────────────────────
      Enemy->ApplyFinisher(MeleeDamage, GetController(), this);
    }
    else
    {
      // ── Coup normal : dégâts de base + petit stun ────────────────────────
      UGameplayStatics::ApplyDamage(Target, MeleeDamage, GetController(), this, nullptr);

      if (Enemy)
        Enemy->ApplyStun(0.6f);
    }
  }
}

// =============================================================================
// IFRAMES
// =============================================================================

float ARevenantOpsCharacter::TakeDamage(float DamageAmount,
                                         const FDamageEvent& DamageEvent,
                                         AController* EventInstigator,
                                         AActor* DamageCauser)
{
  // Invincibilité active : ignorer les dégâts
  if (bInvincible) return 0.f;

  const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

  // Démarrer la fenêtre d'invincibilité
  if (InvincibilityDuration > 0.f && Damage > 0.f)
  {
    bInvincible = true;
    GetWorldTimerManager().SetTimer(
      IFrameTimer, this,
      &ARevenantOpsCharacter::EndInvincibility,
      InvincibilityDuration, false
    );
  }

  return Damage;
}

void ARevenantOpsCharacter::EndInvincibility()
{
  bInvincible = false;
}

// =============================================================================
// WEAPON SYSTEM
// =============================================================================

void ARevenantOpsCharacter::SetDefaultWeaponClasses(
    const TArray<TSubclassOf<AWeaponBase>> &NewClasses) {
  DefaultWeaponClasses = NewClasses;
}

void ARevenantOpsCharacter::SpawnDefaultWeapons() {
  // Destroy existing weapons first
  for (AWeaponBase *Weapon : WeaponInventory) {
    if (Weapon) {
      Weapon->Destroy();
    }
  }
  WeaponInventory.Empty();
  CurrentWeapon = nullptr;
  CurrentWeaponIndex = 0;

  for (const TSubclassOf<AWeaponBase> &WeaponClass : DefaultWeaponClasses) {
    if (!WeaponClass) {
      continue;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    AWeaponBase *NewWeapon =
        GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, SpawnParams);

    if (NewWeapon) {
      NewWeapon->SetOwnerPawn(this);

      // Appliquer le multiplicateur de capacité de munitions du personnage
      if (AmmoCapacityMultiplier != 1.f)
      {
        int32 NewMax = FMath::RoundToInt(NewWeapon->GetMaxReserveAmmo() * AmmoCapacityMultiplier);
        NewWeapon->SetMaxReserveAmmo(NewMax);
      }

      WeaponInventory.Add(NewWeapon);

      // Hide all weapons initially (EquipWeapon will show the active one)
      NewWeapon->SetActorHiddenInGame(true);
      NewWeapon->SetActorTickEnabled(false);
    }
  }

  // Equip the first weapon
  if (WeaponInventory.Num() > 0) {
    bIsArmed = true;
    EquipWeapon(0);
  }

  // Sync weapon slots into RE5 inventory (slots 0 and 1)
  for (int32 i = 0; i < WeaponInventory.Num() && i < 2; ++i) {
    if (!WeaponInventory[i]) continue;
    FInventoryItem& Slot = Inventory[i];
    Slot.Type        = EInventoryItemType::Weapon;
    Slot.DisplayName = WeaponInventory[i]->GetWeaponName();
    Slot.Description = FText::FromString(TEXT("Arme equipee"));
    Slot.Quantity    = 1;
    Slot.WeaponClass = WeaponInventory[i]->GetClass();
    Slot.ItemIcon    = WeaponInventory[i]->GetWeaponIcon();
  }

  // ── RE5 : munitions en inventaire seulement via StartingItems / pickups ─────
  // SpawnDefaultWeapons ne crée plus de slots munitions automatiquement.
  // Les armes démarrent avec leur chargeur plein, sans réserve.
  // La réserve vient de InitStartingInventory (DA_Item_Ammo_*) ou de pickups au sol.
  for (AWeaponBase* Weapon : WeaponInventory)
  {
    if (Weapon) Weapon->SetMaxReserveAmmo(0); // réserve dans l'inventaire, pas dans l'arme
  }
}

bool ARevenantOpsCharacter::AddItemToInventory(const FInventoryItem& Item) {
  for (FInventoryItem& Slot : Inventory) {
    if (Slot.IsEmpty()) {
      Slot = Item;
      return true;
    }
  }
  return false; // Inventory full
}

void ARevenantOpsCharacter::UseInventoryItem(int32 SlotIndex) {
  if (!Inventory.IsValidIndex(SlotIndex)) return;
  FInventoryItem& Item = Inventory[SlotIndex];
  if (Item.IsEmpty()) return;

  switch (Item.Type) {
    case EInventoryItemType::Health:
      if (HealthComp) {
        // Si HealAmount non configuré dans le DA, fallback 25% HP max
        const float Amount = (Item.HealAmount > 0.f)
            ? Item.HealAmount
            : HealthComp->GetMaxHealth() * 0.25f;
        HealthComp->Heal(Amount);
      }
      Item = FInventoryItem(); // consomme le slot
      break;

    case EInventoryItemType::TimeBonus:
      // Time bonus applied by PlayerController via delegate
      // Item stays — controller will read TimeBonusSeconds and clear it
      break;

    case EInventoryItemType::Weapon:
      // Equip weapon if it matches a slot in WeaponInventory
      for (int32 i = 0; i < WeaponInventory.Num(); ++i) {
        if (WeaponInventory[i] && WeaponInventory[i]->GetClass() == Item.WeaponClass) {
          EquipWeapon(i);
          break;
        }
      }
      break;

    default:
      break;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Drop inventaire → spawn monde
// ─────────────────────────────────────────────────────────────────────────────

void ARevenantOpsCharacter::DropInventoryItem(int32 SlotIndex)
{
    if (!Inventory.IsValidIndex(SlotIndex)) return;
    const FInventoryItem Item = Inventory[SlotIndex];
    if (Item.IsEmpty()) return;

    // Les armes ne peuvent pas être jetées
    if (Item.Type == EInventoryItemType::Weapon) return;

    // Spawn devant le joueur, légèrement décalé
    const FVector Offset   = GetActorForwardVector() * 120.f + FVector(0.f, 0.f, 20.f);
    const FVector SpawnLoc = GetActorLocation() + Offset;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AInventoryDropPickup* Drop = GetWorld()->SpawnActor<AInventoryDropPickup>(
        AInventoryDropPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);

    if (Drop)
    {
        Drop->Initialize(Item);
    }

    // Vider le slot
    Inventory[SlotIndex] = FInventoryItem();
}

// ─────────────────────────────────────────────────────────────────────────────
// Combine inventaire style RE5
// ─────────────────────────────────────────────────────────────────────────────

bool ARevenantOpsCharacter::CombineInventoryItems(int32 SlotA, int32 SlotB)
{
    if (SlotA == SlotB) return false;
    if (!Inventory.IsValidIndex(SlotA) || !Inventory.IsValidIndex(SlotB)) return false;

    FInventoryItem& A = Inventory[SlotA];
    FInventoryItem& B = Inventory[SlotB];
    if (A.IsEmpty() || B.IsEmpty()) return false;

    // ── Armes : jamais combinables ───────────────────────────────────────────
    if (A.Type == EInventoryItemType::Weapon || B.Type == EInventoryItemType::Weapon)
        return false;

    // ── Soin + Soin ──────────────────────────────────────────────────────────
    if (A.Type == EInventoryItemType::Health && B.Type == EInventoryItemType::Health)
    {
        const float MaxHP  = HealthComp ? HealthComp->GetMaxHealth() : 100.f;

        // HealAmount = 0 → fallback 25% max
        const float HealA  = (A.HealAmount > 0.f) ? A.HealAmount : MaxHP * 0.25f;
        const float HealB  = (B.HealAmount > 0.f) ? B.HealAmount : MaxHP * 0.25f;
        const float Total  = FMath::Min(HealA + HealB, MaxHP);

        // Tier visuel selon le total
        FString TierName;
        if (Total >= MaxHP * 0.85f)      TierName = TEXT("Grand Soin");
        else if (Total >= MaxHP * 0.45f) TierName = TEXT("Soin Moyen");
        else                              TierName = TEXT("Petit Soin");

        A.HealAmount  = Total;
        A.DisplayName = FText::FromString(TierName);
        // Garde l'icône de A (ou de B si A n'en a pas)
        if (!A.ItemIcon && B.ItemIcon) A.ItemIcon = B.ItemIcon;

        B = FInventoryItem(); // consomme le slot B
        return true;
    }

    // ── Munitions + Munitions (même type) → empilage ─────────────────────────
    if (A.Type == EInventoryItemType::Ammo && B.Type == EInventoryItemType::Ammo
        && A.AmmoType == B.AmmoType)
    {
        A.Quantity += B.Quantity;
        B = FInventoryItem();
        return true;
    }

    // ── Munitions + Arme (inventaire) → recharge chargeur ───────────────────
    // Déterminer qui est l'ammo et qui est l'arme
    const bool AisAmmo   = (A.Type == EInventoryItemType::Ammo);
    const bool BisAmmo   = (B.Type == EInventoryItemType::Ammo);
    const bool AisWeapon = (A.Type == EInventoryItemType::Weapon);
    const bool BisWeapon = (B.Type == EInventoryItemType::Weapon);

    // (Cette branche ne sera jamais atteinte car on bloque les Weapon ci-dessus,
    //  mais on la garde si on change la règle plus tard)
    if ((AisAmmo && BisWeapon) || (BisAmmo && AisWeapon))
    {
        FInventoryItem* AmmoItem   = AisAmmo   ? &A : &B;
        FInventoryItem* WeaponItem = AisWeapon ? &A : &B;
        const int32     AmmoSlot   = AisAmmo   ? SlotA : SlotB;

        for (AWeaponBase* Weapon : WeaponInventory)
        {
            if (!Weapon) continue;
            if (WeaponItem->WeaponClass && Weapon->GetClass() != WeaponItem->WeaponClass) continue;
            if (Weapon->GetWeaponAmmoType() != AmmoItem->AmmoType) continue;

            const int32 Added = Weapon->AddDirectlyToMagazine(AmmoItem->Quantity);
            AmmoItem->Quantity -= Added;
            if (AmmoItem->Quantity <= 0) Inventory[AmmoSlot] = FInventoryItem();
            return Added > 0;
        }
        return false;
    }

    // ── Munitions (inventaire) + Arme équipée courante ───────────────────────
    // Si on combine un slot Ammo avec le slot Weapon de l'arme équipée :
    // → recharge directe du chargeur de l'arme équipée.
    if (AisAmmo && !B.IsEmpty() && B.Type == EInventoryItemType::Ammo)
        return false; // déjà géré ci-dessus

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Inventaire de départ (FStartingItem → FInventoryItem)
// ─────────────────────────────────────────────────────────────────────────────

void ARevenantOpsCharacter::InitStartingInventory(const TArray<FStartingItem>& Items)
{
    for (const FStartingItem& Entry : Items)
    {
        if (!Entry.Definition || !Entry.Definition->IsValid()) continue;

        // Les armes sont gérées par SpawnDefaultWeapons — pas de doublon
        if (Entry.Definition->ItemType == EInventoryItemType::Weapon) continue;

        // Pour les munitions : empiler dans le slot existant via AddInventoryAmmo
        if (Entry.Definition->ItemType == EInventoryItemType::Ammo &&
            Entry.Definition->AmmoType != EAmmoType::None)
        {
            AddInventoryAmmo(
                Entry.Definition->AmmoType,
                Entry.Quantity,
                Entry.Definition->ItemIcon.Get(),
                Entry.Definition->DisplayName);
            continue;
        }

        // Tous les autres types (Health, TimeBonus) : slot normal
        FInventoryItem NewItem = Entry.Definition->MakeInventoryItem(Entry.Quantity);
        AddItemToInventory(NewItem);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Système munitions RE5
// ─────────────────────────────────────────────────────────────────────────────

int32 ARevenantOpsCharacter::GetInventoryAmmo(EAmmoType Type) const
{
  if (Type == EAmmoType::None) return 0;
  for (const FInventoryItem& Item : Inventory)
  {
    if (Item.Type == EInventoryItemType::Ammo && Item.AmmoType == Type)
      return Item.Quantity;
  }
  return 0;
}

int32 ARevenantOpsCharacter::ConsumeInventoryAmmo(EAmmoType Type, int32 Amount)
{
  if (Type == EAmmoType::None || Amount <= 0) return 0;
  for (FInventoryItem& Item : Inventory)
  {
    if (Item.Type == EInventoryItemType::Ammo && Item.AmmoType == Type)
    {
      const int32 Taken = FMath::Min(Amount, Item.Quantity);
      Item.Quantity -= Taken;
      if (Item.Quantity <= 0)
        Item = FInventoryItem(); // vide le slot
      return Taken;
    }
  }
  return 0;
}

void ARevenantOpsCharacter::AddInventoryAmmo(EAmmoType Type, int32 Amount,
                                              UTexture2D* Icon, FText Name, int32 MaxAmount)
{
  if (Type == EAmmoType::None || Amount <= 0) return;

  // Chercher un slot existant du même type pour empiler
  for (FInventoryItem& Item : Inventory)
  {
    if (Item.Type == EInventoryItemType::Ammo && Item.AmmoType == Type)
    {
      Item.Quantity = (MaxAmount > 0)
                    ? FMath::Min(Item.Quantity + Amount, MaxAmount)
                    : Item.Quantity + Amount;
      return;
    }
  }

  // Pas de slot existant — créer un nouveau
  FInventoryItem NewItem;
  NewItem.Type        = EInventoryItemType::Ammo;
  NewItem.AmmoType    = Type;
  NewItem.Quantity    = (MaxAmount > 0) ? FMath::Min(Amount, MaxAmount) : Amount;
  NewItem.ItemIcon    = Icon;
  NewItem.DisplayName = Name.IsEmpty() ? FText::FromString(TEXT("Munitions")) : Name;
  AddItemToInventory(NewItem);
}

void ARevenantOpsCharacter::UpdateAnimationValues()
{
  if (!GetController()) return;

  // --- Aim Pitch / Yaw (control rotation relative to actor) ---
  const FRotator ControlRot = GetControlRotation();
  const FRotator ActorRot   = GetActorRotation();
  FRotator Delta = (ControlRot - ActorRot).GetNormalized();

  // Pitch: UE stores pitch inverted for controllers — negate for natural up/down
  AimPitch = FMath::ClampAngle(-Delta.Pitch, -90.f, 90.f);
  AimYaw   = FMath::ClampAngle( Delta.Yaw,  -180.f, 180.f);

  // --- Movement Direction (velocity relative to actor forward) ---
  const FVector Velocity = GetVelocity();
  if (Velocity.SizeSquared2D() > 1.f)
  {
    const FVector LocalVel = ActorRot.UnrotateVector(Velocity);
    MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(LocalVel.Y, LocalVel.X));
  }
  else
  {
    MovementDirection = 0.f;
  }
}

void ARevenantOpsCharacter::EquipWeapon(int32 Index) {
  if (!WeaponInventory.IsValidIndex(Index)) {
    return;
  }

  // Unequip current weapon
  if (CurrentWeapon) {
    CurrentWeapon->StopFire();
    CurrentWeapon->StopADS();
    CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentWeapon->SetActorHiddenInGame(true);
    CurrentWeapon->SetActorTickEnabled(false);
  }

  // Equip new weapon
  CurrentWeaponIndex = Index;
  CurrentWeapon = WeaponInventory[Index];

  if (CurrentWeapon) {
    CurrentWeapon->SetActorHiddenInGame(false);
    CurrentWeapon->SetActorTickEnabled(true);
    AttachWeaponToSocket(CurrentWeapon);

    // Play equip montage if assigned on the weapon
    if (UAnimMontage* EqMontage = CurrentWeapon->GetEquipMontage())
    {
      PlayAnimMontage(EqMontage);
    }
  }
}

void ARevenantOpsCharacter::AttachWeaponToSocket(AWeaponBase *Weapon) {
  if (!Weapon || !GetMesh()) {
    return;
  }

  Weapon->AttachToComponent(GetMesh(),
                            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                            WeaponAttachSocket);

  // Applique l'offset défini dans le BP de l'arme (position/rotation dans la main)
  const FTransform& Offset = Weapon->GetWeaponAttachOffset();
  Weapon->SetActorRelativeTransform(Offset);
}

// ========== WEAPON INPUT HANDLERS ==========

void ARevenantOpsCharacter::FirePressed() {
  if (CurrentWeapon) {
    // Stop sprinting when firing
    if (bIsSprinting) {
      bIsSprinting = false;
      TargetSpeed = WalkSpeed;
    }

    // Orient character to camera direction when firing
    if (!bIsAiming) {
      GetCharacterMovement()->bOrientRotationToMovement = false;
      bUseControllerRotationYaw = true;
    }

    CurrentWeapon->StartFire();
  }
}

void ARevenantOpsCharacter::FireReleased() {
  if (CurrentWeapon) {
    CurrentWeapon->StopFire();

    // Restore rotation mode if not aiming
    if (!bIsAiming) {
      GetCharacterMovement()->bOrientRotationToMovement = true;
      bUseControllerRotationYaw = false;
    }
  }
}

void ARevenantOpsCharacter::AimPressed() {
  bIsAiming = true;

  // Lock rotation to camera
  GetCharacterMovement()->bOrientRotationToMovement = false;
  bUseControllerRotationYaw = true;

  // Stop sprinting
  if (bIsSprinting) {
    bIsSprinting = false;
    TargetSpeed = WalkSpeed;
  }

  if (CurrentWeapon) {
    CurrentWeapon->StartADS();
  }
}

void ARevenantOpsCharacter::AimReleased() {
  bIsAiming = false;

  // Always restore free rotation when releasing aim
  GetCharacterMovement()->bOrientRotationToMovement = true;
  bUseControllerRotationYaw = false;

  if (CurrentWeapon) {
    CurrentWeapon->StopADS();
  }
}

void ARevenantOpsCharacter::ReloadPressed() {
  if (CurrentWeapon) {
    CurrentWeapon->StartReload();
  }
}

void ARevenantOpsCharacter::AddAndEquipWeapon(AWeaponBase* NewWeapon)
{
    if (!NewWeapon) return;

    NewWeapon->SetOwnerPawn(this);
    WeaponInventory.Add(NewWeapon);
    NewWeapon->SetActorHiddenInGame(true);
    NewWeapon->SetActorTickEnabled(false);

    bIsArmed = true;
    EquipWeapon(WeaponInventory.Num() - 1);
}

void ARevenantOpsCharacter::SwitchWeaponPressed() {
  if (WeaponInventory.Num() <= 1) {
    return;
  }

  const int32 NextIndex = (CurrentWeaponIndex + 1) % WeaponInventory.Num();
  EquipWeapon(NextIndex);
}

// =============================================================================
// INTERACT / PICKUP
// =============================================================================

void ARevenantOpsCharacter::InteractPressed() {
  if (PendingWeaponPickup) {
    PendingWeaponPickup->TryPickup(this);
  } else if (PendingInteractable &&
             PendingInteractable->Implements<UPickupInterface>()) {
    IPickupInterface::Execute_TryPickupInteract(PendingInteractable, this);
  }
}

void ARevenantOpsCharacter::ShowPickupPrompt(UTexture2D* Icon, const FText& Name, int32 Qty) {
  if (ARevenantOpsPlayerController* ROPC = Cast<ARevenantOpsPlayerController>(GetController())) {
    if (URevenantOpsHUD* HUD = ROPC->GetHUDWidget()) {
      HUD->ShowPickupPrompt(Icon, Name, Qty);
    }
  }
}

void ARevenantOpsCharacter::HidePickupPrompt() {
  if (ARevenantOpsPlayerController* ROPC = Cast<ARevenantOpsPlayerController>(GetController())) {
    if (URevenantOpsHUD* HUD = ROPC->GetHUDWidget()) {
      HUD->HidePickupPrompt();
    }
  }
}

// =============================================================================
// DAMAGE FEEDBACK
// =============================================================================

void ARevenantOpsCharacter::OnDamageReceived(UHealthComponent *Comp,
                                              float Health, float HealthDelta,
                                              const AController *InstigatedBy) {
  // Only react to actual damage (negative delta)
  if (HealthDelta >= 0.f) {
    return;
  }

  APlayerController *PC = Cast<APlayerController>(GetController());
  if (!PC) {
    return;
  }

  // Camera shake on taking damage
  PC->ClientStartCameraShake(UCS_TakeDamage::StaticClass(), 1.0f);

  // Damage direction indicator
  if (InstigatedBy) {
    if (APawn *DamagePawn = InstigatedBy->GetPawn()) {
      if (ARevenantOpsPlayerController *ROPC =
              Cast<ARevenantOpsPlayerController>(PC)) {
        if (URevenantOpsHUD *HUD = ROPC->GetHUDWidget()) {
          HUD->ShowDamageDirection(DamagePawn->GetActorLocation());
        }
      }
    }
  }
}
