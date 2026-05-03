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
#include "Gameplay/PickupInterface.h"
#include "Gameplay/WeaponPickup.h"

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

  // Camera boom (over-the-shoulder RE4 style)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 280.f;
  CameraBoom->SocketOffset = FVector(0.f, 65.f, 55.f);
  CameraBoom->bUsePawnControlRotation = true;
  CameraBoom->bDoCollisionTest = true;
  CameraBoom->ProbeSize = 8.f;
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

  // Default OTS offsets — RE5 style over-shoulder
  const FVector HipOffset(0.f, 65.f, 55.f);
  const FVector ADSOffset(0.f, 50.f, 60.f);
  const float HipArmLength = 280.f;
  const float ADSArmLength = 180.f;

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
        HealthComp->Heal(Item.HealAmount);
      }
      Item = FInventoryItem(); // consume
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
