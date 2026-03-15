// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "RevenantOpsCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  RevenantOps base third-person character.
 *  Features: smooth sprint, crouch, slide, dodge, stamina system.
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

  /** Is the character currently sprinting */
  UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint")
  bool bIsSprinting = false;

  /** Is the sprint input currently held */
  bool bWantsToSprint = false;

  // ========== SLIDE ==========

  /** Is the character currently sliding */
  UPROPERTY(BlueprintReadOnly, Category = "Movement|Slide")
  bool bIsSliding = false;

  /** Initial boost speed when entering a slide */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 3000))
  float SlideBoostSpeed = 1200.f;

  /** Ground friction override while sliding */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 10))
  float SlideGroundFriction = 0.5f;

  /** Braking deceleration while sliding */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 5000))
  float SlideBrakingDeceleration = 800.f;

  /** Speed below which the slide ends automatically */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 1000))
  float SlideMinSpeed = 200.f;

  /** Maximum duration of a slide in seconds */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0.1, ClampMax = 3.0))
  float SlideMaxDuration = 1.2f;

  /** Stamina cost of a slide */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide",
            meta = (ClampMin = 0, ClampMax = 100))
  float SlideCost = 20.f;

  /** Slide timer */
  FTimerHandle SlideTimerHandle;

  /** Saved ground friction to restore after slide */
  float DefaultGroundFriction = 8.f;

  /** Saved braking deceleration to restore after slide */
  float DefaultBrakingDeceleration = 2000.f;

  // ========== DODGE / ROLL ==========

  /** Is the character currently dodging */
  UPROPERTY(BlueprintReadOnly, Category = "Movement|Dodge")
  bool bIsDodging = false;

  /** AnimMontage to play for dodge/roll (assign in Blueprint) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge")
  UAnimMontage *DodgeMontage;

  /** Distance of the dodge launch */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge",
            meta = (ClampMin = 0, ClampMax = 2000))
  float DodgeLaunchForce = 800.f;

  /** Cooldown between dodges in seconds */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge",
            meta = (ClampMin = 0, ClampMax = 3))
  float DodgeCooldown = 0.6f;

  /** Stamina cost of a dodge */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dodge",
            meta = (ClampMin = 0, ClampMax = 100))
  float DodgeCost = 25.f;

  /** Timestamp of last dodge for cooldown */
  float LastDodgeTime = -100.f;

  /** Dodge montage ended delegate */
  FOnMontageEnded OnDodgeMontageEnded;

  // ========== STAMINA ==========

  /** Maximum stamina */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 200))
  float MaxStamina = 100.f;

  /** Current stamina */
  UPROPERTY(BlueprintReadOnly, Category = "Movement|Stamina")
  float CurrentStamina = 100.f;

  /** Stamina drain per second while sprinting */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 100))
  float SprintStaminaDrain = 15.f;

  /** Stamina regeneration per second (when not draining) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 100))
  float StaminaRegenRate = 20.f;

  /** Delay before stamina starts regenerating after last drain */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 5))
  float StaminaRegenDelay = 1.0f;

  /** Timestamp of last stamina drain */
  float LastStaminaDrainTime = -100.f;

  /** If true, stamina is depleted and sprint is locked until partial regen */
  UPROPERTY(BlueprintReadOnly, Category = "Movement|Stamina")
  bool bStaminaDepleted = false;

  /** Stamina threshold to recover from depleted state */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina",
            meta = (ClampMin = 0, ClampMax = 100))
  float StaminaRecoveryThreshold = 20.f;

  // ========== CAMERA ==========

  /** Default FOV */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
            meta = (ClampMin = 60, ClampMax = 130))
  float DefaultFOV = 90.f;

  /** Sprint FOV (wider for speed feeling) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
            meta = (ClampMin = 60, ClampMax = 130))
  float SprintFOV = 100.f;

  /** FOV interpolation speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
            meta = (ClampMin = 1.0, ClampMax = 20.0))
  float FOVInterpSpeed = 6.f;

  /** Target speed to send to the Tick interpolation */
  float TargetSpeed = 500.f;

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

  // ========== LOCOMOTION LOGIC ==========

  /** Updates speed interpolation each frame */
  void UpdateMovementSpeed(float DeltaTime);

  /** Updates stamina drain and regen each frame */
  void UpdateStamina(float DeltaTime);

  /** Updates camera FOV each frame */
  void UpdateCameraFOV(float DeltaTime);

  /** Consumes stamina and records drain time */
  bool ConsumeStamina(float Amount);

  /** Starts a slide (called when crouching while sprinting) */
  void StartSlide();

  /** Ends the current slide */
  void EndSlide();

  /** Starts a dodge/roll */
  void StartDodge();

  /** Called when the dodge montage ends */
  void DodgeMontageEnded(UAnimMontage *Montage, bool bInterrupted);

public:
  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoMove(float Right, float Forward);

  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoLook(float Yaw, float Pitch);

  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoJumpStart();

  UFUNCTION(BlueprintCallable, Category = "Input")
  virtual void DoJumpEnd();

  /** Returns current stamina as a 0-1 percentage */
  UFUNCTION(BlueprintCallable, Category = "Movement|Stamina")
  float GetStaminaPercent() const;

  /** Returns true if the character is currently sprinting */
  UFUNCTION(BlueprintCallable, Category = "Movement")
  bool IsSprinting() const { return bIsSprinting; }

  /** Returns true if the character is currently sliding */
  UFUNCTION(BlueprintCallable, Category = "Movement")
  bool IsSliding() const { return bIsSliding; }

  /** Returns true if the character is currently dodging */
  UFUNCTION(BlueprintCallable, Category = "Movement")
  bool IsDodging() const { return bIsDodging; }

public:
  FORCEINLINE class USpringArmComponent *GetCameraBoom() const {
    return CameraBoom;
  }
  FORCEINLINE class UCameraComponent *GetFollowCamera() const {
    return FollowCamera;
  }
};
