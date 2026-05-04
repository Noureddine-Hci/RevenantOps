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
#include "RevenantOpsCharacter.h"
#include "Gameplay/MercenairesGameState.h"
#include "RevenantOpsPlayerController.h"
#include "UI/RevenantOpsHUD.h"

AEnemyBase::AEnemyBase() {
  PrimaryActorTick.bCanEverTick = true;

  // AI Controller auto-possession
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

  // Don't rotate with controller
  bUseControllerRotationYaw = false;
  GetCharacterMovement()->bUseControllerDesiredRotation = false;
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

  // Force capsule and mesh to block Visibility traces (weapon hitscan)
  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
  GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

  UE_LOG(LogTemp, Warning, TEXT("[EnemyBase] %s BeginPlay — Capsule collision=%d, Visibility response=%d"),
      *GetName(),
      (int32)GetCapsuleComponent()->GetCollisionEnabled(),
      (int32)GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Visibility));

  // Bind health events
  if (HealthComp) {
    HealthComp->OnDeath.AddDynamic(this, &AEnemyBase::HandleDeath);
    HealthComp->OnHealthChanged.AddDynamic(this, &AEnemyBase::HandleDamage);
  }

  // Spawn weapon
  SpawnWeapon();

  // Set initial state
  AlertState = EEnemyAlertState::Idle;

  // Create dynamic material instances for hit flash
  if (GetMesh()) {
    for (int32 i = 0; i < GetMesh()->GetNumMaterials(); ++i) {
      UMaterialInstanceDynamic *MID =
          GetMesh()->CreateAndSetMaterialInstanceDynamic(i);
      if (MID) {
        HitFlashMaterials.Add(MID);
      }
    }
  }
}

void AEnemyBase::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (HealthComp && HealthComp->IsDead()) {
    return;
  }

  UpdatePerception(DeltaTime);
  UpdateCombat(DeltaTime);

  // Forcer la rotation vers la direction de déplacement
  if (!bIsDead)
  {
    FVector Vel = GetVelocity();
    Vel.Z = 0.f;
    if (!Vel.IsNearlyZero(1.f))
    {
      SetActorRotation(FRotator(0.f, Vel.Rotation().Yaw, 0.f));
    }
  }

  // Hit flash decay
  if (HitFlashTimer > 0.f) {
    HitFlashTimer -= DeltaTime;
    const float Alpha =
        FMath::Clamp(HitFlashTimer / HitFlashDuration, 0.f, 1.f);
    for (UMaterialInstanceDynamic *MID : HitFlashMaterials) {
      if (MID) {
        MID->SetScalarParameterValue(FName("HitFlash"), Alpha);
      }
    }
  }

  // Ambient grunt sounds
  if (AmbientSound && AlertState == EEnemyAlertState::Alert) {
    AmbientSoundTimer -= DeltaTime;
    if (AmbientSoundTimer <= 0.f) {
      UGameplayStatics::PlaySoundAtLocation(this, AmbientSound,
                                             GetActorLocation());
      AmbientSoundTimer =
          AmbientSoundInterval + FMath::FRandRange(-1.f, 2.f);
    }
  }
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
  if (bIsDead) {
    return;
  }
  bIsDead = true;

  // Stop firing
  if (EquippedWeapon) {
    EquippedWeapon->StopFire();
  }

  // Disable movement
  GetCharacterMovement()->DisableMovement();

  // Disable collision
  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // ── Death animation → ragdoll différé ────────────────────────────────────
  // Si une DeathAnim est assignée, on la joue d'abord puis on passe en ragdoll
  // après sa durée. Sinon ragdoll immédiat.
  float RagdollDelay = 0.f;
  if (DeathAnim)
  {
    if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
    {
      UAnimMontage* DynMontage = AI->PlaySlotAnimationAsDynamicMontage(
          DeathAnim, FName("DefaultSlot"),
          /*BlendIn=*/0.1f, /*BlendOut=*/0.f,
          /*PlayRate=*/1.f);
      if (DynMontage)
      {
        RagdollDelay = DynMontage->GetPlayLength();
      }
    }
  }

  if (RagdollDelay > 0.f)
  {
    // Ragdoll après la fin de l'anim
    GetWorldTimerManager().SetTimer(
        DeathRagdollTimer,
        [this]()
        {
          if (IsValid(this) && GetMesh())
          {
            GetMesh()->SetSimulatePhysics(true);
            GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
          }
        },
        RagdollDelay, /*bLoop=*/false);
  }
  else
  {
    // Ragdoll immédiat (pas de DeathAnim assignée)
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
  }

  // Hide life bar
  if (LifeBarWidget) {
    LifeBarWidget->SetHiddenInGame(true);
  }

  // Play death sound
  if (DeathSound) {
    UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
  }

  // Register kill in GameState (score + combo) and show kill notification
  if (AMercenairesGameState *GS =
          GetWorld()->GetGameState<AMercenairesGameState>()) {
    // Snapshot combo BEFORE RegisterKill increments it
    const int32 ComboAtKill  = GS->GetComboMultiplier();
    const int32 Points       = GetKillPoints() * ComboAtKill;
    GS->RegisterKill(this);

    // Show "+points  EnemyName" on HUD
    if (APlayerController *PC = GetWorld()->GetFirstPlayerController()) {
      if (ARevenantOpsPlayerController *ROPC =
              Cast<ARevenantOpsPlayerController>(PC)) {
        if (URevenantOpsHUD *HUD = ROPC->GetHUDWidget()) {
          HUD->ShowKillNotification(EnemyName.ToString(), Points);
        }
      }
    }
  }

  // ── Ammo drops adaptatifs ─────────────────────────────────────────────────
  if (AmmoDrop.Num() > 0)
  {
    // Collecter les types d'armes que le joueur possède actuellement
    TSet<EAmmoType> PlayerAmmoTypes;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
      if (ARevenantOpsCharacter* MercChar = Cast<ARevenantOpsCharacter>(PC->GetPawn()))
      {
        // Arme active
        if (AWeaponBase* W = MercChar->GetCurrentWeapon())
          PlayerAmmoTypes.Add(W->GetWeaponAmmoType());
        // On pourrait aussi boucler sur toutes les armes — ici arme active suffit
        // pour ne dropper que ce qui est utile immédiatement
      }
    }

    for (const FAmmoDropEntry& Drop : AmmoDrop)
    {
      if (Drop.DropChance <= 0.f || !Drop.DropClass) continue;

      // Ne dropper que si le joueur a une arme de ce type
      if (!PlayerAmmoTypes.Contains(Drop.AmmoType)) continue;

      if (FMath::FRand() <= Drop.DropChance)
      {
        FVector SpawnLoc = GetActorLocation() + FVector(
            FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 30.f);
        FActorSpawnParameters SP;
        SP.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        if (AAmmoBonusPickup* DropActor = GetWorld()->SpawnActor<AAmmoBonusPickup>(
                Drop.DropClass, SpawnLoc, FRotator::ZeroRotator, SP))
        {
          DropActor->AmmoAmount      = Drop.AmmoAmount;
          DropActor->TargetAmmoType  = Drop.AmmoType;
          DropActor->DropLifetime    = Drop.Lifetime;
          DropActor->RespawnTime     = 0.f;
          // ItemIcon et DisplayName viennent automatiquement des defaults du BP
          DropActor->StartLifetimeTimer();
        }
      }
    }
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

  // Trigger hit flash
  HitFlashTimer = HitFlashDuration;

  // Hit react via slot — retourne automatiquement à la locomotion
  if (HitReactAnim && !bIsDead)
  {
    if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
    {
      AI->PlaySlotAnimationAsDynamicMontage(HitReactAnim, FName("DefaultSlot"), 0.05f, 0.1f, 1.f);
    }
  }

  // Play hit sound
  if (HitSound) {
    UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
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
