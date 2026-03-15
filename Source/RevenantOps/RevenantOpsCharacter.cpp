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

ARevenantOpsCharacter::ARevenantOpsCharacter() {
  PrimaryActorTick.bCanEverTick = true;

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
  GetCharacterMovement()->CrouchedHalfHeight = 58.f;

  // Camera boom (over-the-shoulder tactical view)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 50.0f;
  CameraBoom->SocketOffset = FVector(-146.1f, -127.2f, 14.3f);
  CameraBoom->bUsePawnControlRotation = true;

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
}

// =============================================================================
// TICK
// =============================================================================

void ARevenantOpsCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  UpdateMovementSpeed(DeltaTime);
  UpdateStamina(DeltaTime);
  UpdateCameraFOV(DeltaTime);

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
  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

    // Jumping
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
                                       &ACharacter::Jump);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed,
                                       this, &ACharacter::StopJumping);

    // Moving
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
                                       this, &ARevenantOpsCharacter::Move);
    EnhancedInputComponent->BindAction(MouseLookAction,
                                       ETriggerEvent::Triggered, this,
                                       &ARevenantOpsCharacter::Look);

    // Looking
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                       this, &ARevenantOpsCharacter::Look);

    // Sprinting (hold)
    EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered,
                                       this,
                                       &ARevenantOpsCharacter::StartSprint);
    EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed,
                                       this,
                                       &ARevenantOpsCharacter::StopSprint);

    // Crouching (toggle)
    if (CrouchAction) {
      EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started,
                                         this,
                                         &ARevenantOpsCharacter::CrouchPressed);
    }

    // Dodge/Roll
    if (DodgeAction) {
      EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started,
                                         this,
                                         &ARevenantOpsCharacter::DodgePressed);
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

  // No manual movement while sliding (momentum carries the character)
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
  // Cancel slide on jump
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

  // Can't sprint if stamina is depleted, crouching, or sliding
  if (bStaminaDepleted || bIsCrouched || bIsSliding) {
    return;
  }

  bIsSprinting = true;
  TargetSpeed = SprintSpeed;
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
  // Can't crouch while dodging
  if (bIsDodging) {
    return;
  }

  // If sliding, cancel the slide
  if (bIsSliding) {
    EndSlide();
    return;
  }

  if (bIsCrouched) {
    // Uncrouch
    UnCrouch();
    // Restore sprint if still holding sprint
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
      // Normal crouch
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
  // Check stamina
  if (!ConsumeStamina(SlideCost)) {
    // Not enough stamina, just crouch instead
    Crouch();
    bIsSprinting = false;
    TargetSpeed = CrouchMoveSpeed;
    return;
  }

  bIsSliding = true;
  bIsSprinting = false;

  // Enter crouch state for the lower capsule
  Crouch();

  // Override friction for slide momentum
  GetCharacterMovement()->GroundFriction = SlideGroundFriction;
  GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration;
  GetCharacterMovement()->MaxWalkSpeedCrouched = SlideBoostSpeed;

  // Launch the character forward
  const FVector SlideDirection = GetActorForwardVector();
  LaunchCharacter(SlideDirection * SlideBoostSpeed, true, false);

  // Safety timer to end slide after max duration
  GetWorld()->GetTimerManager().SetTimer(
      SlideTimerHandle, this, &ARevenantOpsCharacter::EndSlide,
      SlideMaxDuration, false);
}

void ARevenantOpsCharacter::EndSlide() {
  if (!bIsSliding) {
    return;
  }

  bIsSliding = false;

  // Clear the safety timer
  GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandle);

  // Restore default movement values
  GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
  GetCharacterMovement()->BrakingDecelerationWalking =
      DefaultBrakingDeceleration;
  GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchMoveSpeed;

  // Stand back up
  UnCrouch();

  // Determine post-slide speed
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
  // Cooldown check
  const float CurrentTime = GetWorld()->GetTimeSeconds();
  if (CurrentTime - LastDodgeTime < DodgeCooldown) {
    return;
  }

  // Can't dodge while already dodging or sliding
  if (bIsDodging || bIsSliding) {
    return;
  }

  // Stamina check
  if (!ConsumeStamina(DodgeCost)) {
    return;
  }

  bIsDodging = true;
  LastDodgeTime = CurrentTime;

  // Determine dodge direction: use movement input or fall back to character
  // forward
  FVector DodgeDirection;
  const FVector InputVector = GetCharacterMovement()->GetLastInputVector();
  if (InputVector.SizeSquared() > 0.1f) {
    DodgeDirection = InputVector.GetSafeNormal();
  } else {
    DodgeDirection = GetActorForwardVector();
  }

  // Launch the character
  LaunchCharacter(DodgeDirection * DodgeLaunchForce, true, false);

  // Play dodge montage if assigned
  if (DodgeMontage) {
    if (UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance()) {
      const float MontageLength = AnimInstance->Montage_Play(DodgeMontage);
      if (MontageLength > 0.f) {
        AnimInstance->Montage_SetEndDelegate(OnDodgeMontageEnded, DodgeMontage);
      } else {
        // Montage failed to play, reset dodge state
        bIsDodging = false;
      }
    }
  } else {
    // No montage assigned - end dodge after a brief delay
    FTimerHandle DodgeEndTimer;
    GetWorld()->GetTimerManager().SetTimer(
        DodgeEndTimer,
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

    // Force stop sprint
    if (bIsSprinting) {
      bIsSprinting = false;
      TargetSpeed = bIsCrouched ? CrouchMoveSpeed : WalkSpeed;
    }
  }

  return true;
}

void ARevenantOpsCharacter::UpdateStamina(float DeltaTime) {
  const float CurrentTime = GetWorld()->GetTimeSeconds();

  // Drain stamina while sprinting
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
  }
  // Regenerate stamina after delay
  else if (CurrentTime - LastStaminaDrainTime >= StaminaRegenDelay) {
    CurrentStamina =
        FMath::Min(CurrentStamina + StaminaRegenRate * DeltaTime, MaxStamina);

    // Recover from depleted state
    if (bStaminaDepleted && CurrentStamina >= StaminaRecoveryThreshold) {
      bStaminaDepleted = false;

      // Re-enable sprint if still holding input
      if (bWantsToSprint && !bIsCrouched && !bIsSliding) {
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
    // During slide, the physics/friction handle deceleration - don't override
    return;
  }

  const float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
  const float NewSpeed =
      FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, SpeedInterpRate);
  GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ARevenantOpsCharacter::UpdateCameraFOV(float DeltaTime) {
  if (!FollowCamera) {
    return;
  }

  const float TargetFOV = bIsSprinting ? SprintFOV : DefaultFOV;
  const float CurrentFOV = FollowCamera->FieldOfView;
  const float NewFOV =
      FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
  FollowCamera->SetFieldOfView(NewFOV);
}
