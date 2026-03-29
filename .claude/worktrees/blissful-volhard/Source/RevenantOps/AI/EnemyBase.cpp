// Copyright RevenantOps. All Rights Reserved.

#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "TimerManager.h"
#include "WeaponBase.h"

AEnemyBase::AEnemyBase() {
  PrimaryActorTick.bCanEverTick = true;

  // AI Controller auto-possession
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

  // Don't rotate with controller
  bUseControllerRotationYaw = false;
  GetCharacterMovement()->bUseControllerDesiredRotation = true;
  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);

  // Default movement
  GetCharacterMovement()->MaxWalkSpeed = 400.f;

  // Health component
  HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

  // Life bar widget
  LifeBarWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBarWidget"));
  LifeBarWidget->SetupAttachment(RootComponent);
  LifeBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
  LifeBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
  LifeBarWidget->SetDrawSize(FVector2D(150.f, 15.f));

  // Enemy tag
  Tags.Add(FName("Enemy"));
}

void AEnemyBase::BeginPlay() {
  Super::BeginPlay();

  // Bind health events
  if (HealthComp) {
    HealthComp->OnDeath.AddDynamic(this, &AEnemyBase::HandleDeath);
    HealthComp->OnHealthChanged.AddDynamic(this, &AEnemyBase::HandleDamage);
  }

  // Spawn weapon
  SpawnWeapon();

  // Set initial state
  AlertState = EEnemyAlertState::Idle;
}

void AEnemyBase::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (HealthComp && HealthComp->IsDead()) {
    return;
  }

  UpdatePerception(DeltaTime);
  UpdateCombat(DeltaTime);
}

// =============================================================================
// WEAPON
// =============================================================================

void AEnemyBase::SpawnWeapon() {
  if (!WeaponClass) {
    return;
  }

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner = this;
  SpawnParams.Instigator = this;

  EquippedWeapon =
      GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, SpawnParams);

  if (EquippedWeapon) {
    EquippedWeapon->SetOwnerPawn(this);
    EquippedWeapon->AttachToComponent(
        GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        WeaponSocket);
  }
}

// =============================================================================
// PERCEPTION
// =============================================================================

bool AEnemyBase::CanSeePlayer() const {
  APawn *Player = UGameplayStatics::GetPlayerPawn(this, 0);
  if (!Player) {
    return false;
  }

  const FVector EyeLocation = GetActorLocation() + FVector(0, 0, 60.f);
  const FVector ToPlayer = Player->GetActorLocation() - EyeLocation;
  const float Distance = ToPlayer.Size();

  // Range check
  if (Distance > SightRange) {
    return false;
  }

  // Cone check
  const FVector Forward = GetActorForwardVector();
  const FVector DirectionToPlayer = ToPlayer.GetSafeNormal();
  const float DotProduct = FVector::DotProduct(Forward, DirectionToPlayer);
  const float AngleCos = FMath::Cos(FMath::DegreesToRadians(SightHalfAngle));

  if (DotProduct < AngleCos) {
    return false;
  }

  // Line of sight check (trace for obstructions)
  FHitResult HitResult;
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(this);

  if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation,
                                           Player->GetActorLocation(),
                                           ECC_Visibility, QueryParams)) {
    // Did we hit the player?
    return HitResult.GetActor() == Player;
  }

  // No obstruction = can see
  return true;
}

void AEnemyBase::UpdatePerception(float DeltaTime) {
  APawn *Player = UGameplayStatics::GetPlayerPawn(this, 0);
  const bool bCanSee = CanSeePlayer();

  switch (AlertState) {
  case EEnemyAlertState::Idle:
    if (bCanSee && Player) {
      // Transition to Alert
      TargetPlayer = Player;
      LastKnownPlayerLocation = Player->GetActorLocation();
      TimeSinceLastSeen = 0.f;
      bHasReacted = false;
      CurrentReactionTimer = ReactionTime;
      AlertState = EEnemyAlertState::Alert;
      BP_OnAlertStateChanged(AlertState);
    } else {
      UpdatePatrol();
    }
    break;

  case EEnemyAlertState::Alert:
    if (bCanSee && Player) {
      TimeSinceLastSeen = 0.f;
      LastKnownPlayerLocation = Player->GetActorLocation();
      TargetPlayer = Player;
    } else {
      TimeSinceLastSeen += DeltaTime;
      if (TimeSinceLastSeen >= LoseSightTime) {
        // Lost sight, go to searching
        AlertState = EEnemyAlertState::Searching;
        TimeSinceLastSeen = 0.f;
        bIsFiring = false;
        if (EquippedWeapon) {
          EquippedWeapon->StopFire();
        }
        BP_OnAlertStateChanged(AlertState);
      }
    }
    break;

  case EEnemyAlertState::Searching:
    if (bCanSee && Player) {
      // Found player again
      TargetPlayer = Player;
      LastKnownPlayerLocation = Player->GetActorLocation();
      TimeSinceLastSeen = 0.f;
      bHasReacted = false;
      CurrentReactionTimer = ReactionTime * 0.5f; // Faster reaction on re-spot
      AlertState = EEnemyAlertState::Alert;
      BP_OnAlertStateChanged(AlertState);
    } else {
      TimeSinceLastSeen += DeltaTime;
      if (TimeSinceLastSeen >= SearchDuration) {
        // Give up, return to idle
        AlertState = EEnemyAlertState::Idle;
        TargetPlayer = nullptr;
        CurrentPatrolIndex = 0;
        BP_OnAlertStateChanged(AlertState);
      } else {
        // Move towards last known position
        if (AAIController *AIC = Cast<AAIController>(GetController())) {
          AIC->MoveToLocation(LastKnownPlayerLocation, 50.f);
        }
      }
    }
    break;

  default:
    break;
  }
}

// =============================================================================
// COMBAT
// =============================================================================

void AEnemyBase::UpdateCombat(float DeltaTime) {
  if (AlertState != EEnemyAlertState::Alert || !TargetPlayer) {
    return;
  }

  // Reaction delay before engaging
  if (!bHasReacted) {
    CurrentReactionTimer -= DeltaTime;
    if (CurrentReactionTimer > 0.f) {
      // Face the player while reacting
      const FVector ToPlayer =
          TargetPlayer->GetActorLocation() - GetActorLocation();
      const FRotator LookRotation = ToPlayer.Rotation();
      if (AAIController *AIC = Cast<AAIController>(GetController())) {
        AIC->SetFocalPoint(TargetPlayer->GetActorLocation());
      }
      return;
    }
    bHasReacted = true;
  }

  // Face the player
  if (AAIController *AIC = Cast<AAIController>(GetController())) {
    AIC->SetFocalPoint(TargetPlayer->GetActorLocation());
  }

  // Move to engagement position
  MoveToEngagementPosition();

  // Fire burst logic
  const float DistanceToPlayer =
      FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

  if (DistanceToPlayer <= MaxEngagementRange) {
    FireBurstTimer -= DeltaTime;

    if (!bIsFiring && FireBurstTimer <= 0.f) {
      // Start a new burst
      bIsFiring = true;
      CurrentBurstShots = 0;
    }

    if (bIsFiring) {
      FireAtPlayer();
      ++CurrentBurstShots;

      if (CurrentBurstShots >= ShotsPerBurst) {
        // End burst
        bIsFiring = false;
        FireBurstTimer = FireBurstInterval;
        if (EquippedWeapon) {
          EquippedWeapon->StopFire();
        }
      }
    }
  }
}

void AEnemyBase::FireAtPlayer() {
  if (!EquippedWeapon || !TargetPlayer) {
    return;
  }

  // Make the AI controller look at the player with accuracy spread
  if (AController *C = GetController()) {
    const FVector TargetLocation = TargetPlayer->GetActorLocation();

    // Apply inaccuracy: spread based on (1 - Accuracy)
    const float InaccuracyRadius = (1.f - Accuracy) * 200.f;
    const FVector Spread =
        FVector(FMath::FRandRange(-InaccuracyRadius, InaccuracyRadius),
                FMath::FRandRange(-InaccuracyRadius, InaccuracyRadius),
                FMath::FRandRange(-InaccuracyRadius * 0.5f,
                                  InaccuracyRadius * 0.5f));

    const FVector AimTarget = TargetLocation + Spread;

    // Set the controller's focal point to the aim target
    if (AAIController *AIC = Cast<AAIController>(C)) {
      AIC->SetFocalPoint(AimTarget);
    }

    EquippedWeapon->StartFire();
  }
}

void AEnemyBase::MoveToEngagementPosition() {
  if (!TargetPlayer) {
    return;
  }

  AAIController *AIC = Cast<AAIController>(GetController());
  if (!AIC) {
    return;
  }

  const float DistanceToPlayer =
      FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

  switch (BehaviorProfile) {
  case EEnemyBehavior::Aggressive:
    // Rush towards the player, but stop at close range
    if (DistanceToPlayer > IdealEngagementRange * 0.5f) {
      AIC->MoveToActor(TargetPlayer, IdealEngagementRange * 0.3f);
    }
    break;

  case EEnemyBehavior::Defensive:
    // Try to maintain ideal range
    if (DistanceToPlayer < IdealEngagementRange * 0.6f) {
      // Too close, back up
      const FVector AwayFromPlayer =
          (GetActorLocation() - TargetPlayer->GetActorLocation())
              .GetSafeNormal();
      const FVector RetreatTarget =
          GetActorLocation() + AwayFromPlayer * 300.f;
      AIC->MoveToLocation(RetreatTarget, 50.f);
    } else if (DistanceToPlayer > IdealEngagementRange * 1.3f) {
      // Too far, close in
      AIC->MoveToActor(TargetPlayer, IdealEngagementRange * 0.8f);
    }
    break;

  case EEnemyBehavior::Flanker:
    // Move to the side of the player
    {
      const FVector ToPlayer =
          (TargetPlayer->GetActorLocation() - GetActorLocation())
              .GetSafeNormal();
      const FVector RightOfPlayer = FVector::CrossProduct(ToPlayer, FVector::UpVector);
      const FVector FlankTarget =
          TargetPlayer->GetActorLocation() +
          RightOfPlayer * IdealEngagementRange * 0.7f;
      AIC->MoveToLocation(FlankTarget, 100.f);
    }
    break;

  case EEnemyBehavior::Sniper:
    // Stay far, don't move much
    if (DistanceToPlayer < IdealEngagementRange * 0.8f) {
      const FVector AwayFromPlayer =
          (GetActorLocation() - TargetPlayer->GetActorLocation())
              .GetSafeNormal();
      const FVector RetreatTarget =
          GetActorLocation() + AwayFromPlayer * 500.f;
      AIC->MoveToLocation(RetreatTarget, 50.f);
    }
    break;

  case EEnemyBehavior::Patrol:
    // In alert mode, behave like defensive
    if (DistanceToPlayer > IdealEngagementRange * 1.3f) {
      AIC->MoveToActor(TargetPlayer, IdealEngagementRange * 0.8f);
    }
    break;
  }
}

// =============================================================================
// PATROL
// =============================================================================

void AEnemyBase::UpdatePatrol() {
  if (PatrolPoints.Num() == 0) {
    return;
  }

  AAIController *AIC = Cast<AAIController>(GetController());
  if (!AIC) {
    return;
  }

  const FVector CurrentTarget = PatrolPoints[CurrentPatrolIndex];
  const float DistanceToTarget =
      FVector::Dist(GetActorLocation(), CurrentTarget);

  if (DistanceToTarget < 100.f) {
    // Reached waypoint, move to next
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
  }

  AIC->MoveToLocation(PatrolPoints[CurrentPatrolIndex], 50.f);
}

// =============================================================================
// ALERTS
// =============================================================================

void AEnemyBase::AlertToLocation(const FVector &Location) {
  if (AlertState == EEnemyAlertState::Idle) {
    AlertState = EEnemyAlertState::Searching;
    LastKnownPlayerLocation = Location;
    TimeSinceLastSeen = 0.f;
    BP_OnAlertStateChanged(AlertState);
  }
}

void AEnemyBase::ReceiveSquadAlert(const FVector &PlayerLocation,
                                    AEnemyBase *Alerter) {
  if (AlertState == EEnemyAlertState::Idle ||
      AlertState == EEnemyAlertState::Searching) {
    LastKnownPlayerLocation = PlayerLocation;

    if (AlertState == EEnemyAlertState::Idle) {
      AlertState = EEnemyAlertState::Suspicious;
      TimeSinceLastSeen = 0.f;

      // Move towards the alert location
      if (AAIController *AIC = Cast<AAIController>(GetController())) {
        AIC->MoveToLocation(PlayerLocation, 100.f);
      }

      BP_OnAlertStateChanged(AlertState);
    }
  }
}

// =============================================================================
// DEATH & DAMAGE
// =============================================================================

void AEnemyBase::HandleDeath(UHealthComponent *HealthComponent,
                              const AController *InstigatedBy,
                              AActor *DamageCauser) {
  // Stop firing
  if (EquippedWeapon) {
    EquippedWeapon->StopFire();
  }

  // Disable movement
  GetCharacterMovement()->DisableMovement();

  // Disable collision
  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // Enable ragdoll
  GetMesh()->SetSimulatePhysics(true);
  GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

  // Hide life bar
  if (LifeBarWidget) {
    LifeBarWidget->SetHiddenInGame(true);
  }

  // Broadcast death
  OnEnemyDied.Broadcast(this, const_cast<AController *>(InstigatedBy));

  // BP hook for death effects
  BP_OnDeath();

  // Schedule cleanup
  GetWorld()->GetTimerManager().SetTimer(DeathCleanupTimer, this,
                                         &AEnemyBase::DeathCleanup,
                                         DeathCleanupTime, false);
}

void AEnemyBase::HandleDamage(UHealthComponent *HealthComponent, float Health,
                               float HealthDelta,
                               const AController *InstigatedBy) {
  if (HealthDelta >= 0.f) {
    return; // Not damage, it's healing
  }

  // If idle, become alert when taking damage
  if (AlertState == EEnemyAlertState::Idle ||
      AlertState == EEnemyAlertState::Searching) {
    APawn *Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (Player) {
      TargetPlayer = Player;
      LastKnownPlayerLocation = Player->GetActorLocation();
      AlertState = EEnemyAlertState::Alert;
      bHasReacted = false;
      CurrentReactionTimer = ReactionTime * 0.3f; // Quick reaction when shot
      BP_OnAlertStateChanged(AlertState);
    }
  }

  // BP hook
  if (InstigatedBy && InstigatedBy->GetPawn()) {
    const FVector DamageDirection =
        (GetActorLocation() - InstigatedBy->GetPawn()->GetActorLocation())
            .GetSafeNormal();
    BP_OnDamageReceived(FMath::Abs(HealthDelta), DamageDirection);
  }
}

void AEnemyBase::DeathCleanup() { Destroy(); }
