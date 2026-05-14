// Copyright RevenantOps. All Rights Reserved.

#include "EnemyBase.h"
#include "Engine/DamageEvents.h"
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
#include "Gameplay/InventoryDropPickup.h"
#include "RevenantOpsPlayerController.h"
#include "UI/RevenantOpsHUD.h"

AEnemyBase::AEnemyBase() {
  PrimaryActorTick.bCanEverTick = true;

  // Tag utilisé par le joueur pour la détection de mêlée
  Tags.Add(FName("Enemy"));

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
    // Collecter les types de toutes les armes du loadout du joueur
    TSet<EAmmoType> PlayerAmmoTypes;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
      if (ARevenantOpsCharacter* MercChar = Cast<ARevenantOpsCharacter>(PC->GetPawn()))
      {
        for (AWeaponBase* W : MercChar->GetWeaponInventory())
          if (W) PlayerAmmoTypes.Add(W->GetWeaponAmmoType());
      }
    }

    for (const FAmmoDropEntry& Drop : AmmoDrop)
    {
      if (Drop.DropChance <= 0.f) continue;

      // Résoudre le type effectif (DA prioritaire)
      const EAmmoType EffectiveType = (Drop.ItemDefinition && Drop.ItemDefinition->AmmoType != EAmmoType::None)
          ? Drop.ItemDefinition->AmmoType : Drop.AmmoType;

      // Ne dropper que si le joueur a une arme de ce type
      if (!PlayerAmmoTypes.Contains(EffectiveType)) continue;

      if (FMath::FRand() > Drop.DropChance) continue;

      FVector SpawnLoc = GetActorLocation() + FVector(
          FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 30.f);
      FActorSpawnParameters SP;
      SP.SpawnCollisionHandlingOverride =
          ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

      // Résoudre icône et nom (ItemDefinition prioritaire)
      UTexture2D* Icon     = nullptr;
      FText       DispName = Drop.DropDisplayName;

      if (Drop.ItemDefinition)
      {
        Icon = Drop.ItemDefinition->ItemIcon.Get();
        if (!Drop.ItemDefinition->DisplayName.IsEmpty())
          DispName = Drop.ItemDefinition->DisplayName;
      }
      else if (Drop.DropIcon)
      {
        Icon = Drop.DropIcon.Get();
      }

      // Nom fallback lisible si rien n'est configuré
      if (DispName.IsEmpty())
      {
        static const TMap<EAmmoType, FString> AmmoNames = {
          { EAmmoType::Pistol,  TEXT("Munitions Pistolet")      },
          { EAmmoType::Rifle,   TEXT("Munitions Fusil")         },
          { EAmmoType::Shotgun, TEXT("Munitions Fusil à Pompe") },
          { EAmmoType::SMG,     TEXT("Munitions SMG")           },
          { EAmmoType::Sniper,  TEXT("Munitions Sniper")        },
        };
        const FString* Found = AmmoNames.Find(EffectiveType);
        DispName = FText::FromString(Found ? *Found : TEXT("Munitions"));
      }

      // Si un BP pickup custom est assigné → l'utiliser (pour mesh/VFX spécifiques)
      if (Drop.DropClass)
      {
        if (AAmmoBonusPickup* DropActor = GetWorld()->SpawnActor<AAmmoBonusPickup>(
                Drop.DropClass, SpawnLoc, FRotator::ZeroRotator, SP))
        {
          DropActor->AmmoAmount     = Drop.AmmoAmount;
          DropActor->TargetAmmoType = EffectiveType;
          DropActor->DropLifetime   = Drop.Lifetime;
          DropActor->RespawnTime    = 0.f;
          // Passer l'ItemDefinition → icône + nom viennent du DA automatiquement
          if (Drop.ItemDefinition)
            DropActor->ItemDefinition = Drop.ItemDefinition;
          else
          {
            if (Icon)     DropActor->ItemIcon    = Icon;
            if (!DispName.IsEmpty()) DropActor->DisplayName = DispName;
          }
          DropActor->StartLifetimeTimer();
        }
      }
      else
      {
        // Spawn un AInventoryDropPickup générique
        if (AInventoryDropPickup* DropActor = GetWorld()->SpawnActor<AInventoryDropPickup>(
                AInventoryDropPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SP))
        {
          FInventoryItem AmmoItem;
          AmmoItem.Type        = EInventoryItemType::Ammo;
          AmmoItem.AmmoType    = EffectiveType;
          AmmoItem.Quantity    = Drop.AmmoAmount;
          AmmoItem.DisplayName = DispName;
          AmmoItem.ItemIcon    = Icon;
          DropActor->Initialize(AmmoItem);
          DropActor->Lifetime  = Drop.Lifetime;
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

float AEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                              AController* InInstigator, AActor* DamageCauser)
{
  const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, InInstigator, DamageCauser);

  if (bIsDead || Damage <= 0.f) return Damage;

  // ── Damage number flottant sur le HUD ────────────────────────────────────
  {
    const FVector PopupPos = GetActorLocation() + FVector(0.f, 0.f, 90.f); // au-dessus de la tête
    const bool bCrit = DamageEvent.IsOfType(FPointDamageEvent::ClassID) &&
                       static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.BoneName.ToString().ToLower().Contains(TEXT("head"));

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
      if (ARevenantOpsPlayerController* ROPC = Cast<ARevenantOpsPlayerController>(PC))
        if (URevenantOpsHUD* HUD = ROPC->GetHUDWidget())
          HUD->AddDamageNumber(PopupPos, Damage, bCrit);
  }

  // Détecter la zone via le nom d'os (si physics asset présent)
  // + fallback automatique sur la hauteur du point d'impact
  if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
  {
    const FPointDamageEvent& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
    const FName  BoneName  = PointEvent.HitInfo.BoneName;
    const FVector HitPoint = PointEvent.HitInfo.ImpactPoint;

    // Priorité 1 : nom d'os (physics asset présent)
    if (!BoneName.IsNone() && BoneName != NAME_None)
    {
      HandleBoneHit(BoneName);
    }
    else
    {
      // Priorité 2 : position Z relative à la capsule
      // Head  = 80%+ de la hauteur totale
      // Leg   = 0–40% de la hauteur totale
      // Arm   = 55–80% ET hors de l'axe central (côté)
      const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
      const float BaseZ    = GetActorLocation().Z - CapsuleHalfHeight;
      const float FullH    = CapsuleHalfHeight * 2.f;
      const float HitRatio = FMath::Clamp((HitPoint.Z - BaseZ) / FullH, 0.f, 1.f);

      // Distance horizontale du point d'impact par rapport au centre de la capsule
      const FVector2D HitFlat(HitPoint.X, HitPoint.Y);
      const FVector2D ActorFlat(GetActorLocation().X, GetActorLocation().Y);
      const float     SideDist = FVector2D::Distance(HitFlat, ActorFlat);
      const float     CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

      if (HitRatio >= 0.78f)
      {
        HandleBoneHit(FName("head"));
      }
      else if (HitRatio <= 0.40f)
      {
        HandleBoneHit(FName("thigh_l")); // → Leg
      }
      else if (SideDist > CapsuleRadius * 0.5f && HitRatio >= 0.55f)
      {
        HandleBoneHit(FName("upperarm_l")); // → Arm
      }
      // Zone centrale (torse) = pas de finisher
    }
  }

  return Damage;
}

void AEnemyBase::HandleBoneHit(const FName& BoneName)
{
  if (bIsDead) return;

  const FString Bone = BoneName.ToString().ToLower();

  // Tête
  if (Bone.Contains(TEXT("head")) || Bone.Contains(TEXT("neck")))
  {
    OpenFinisherWindow(EFinisherZone::Head);
    return;
  }

  // Bras
  if (Bone.Contains(TEXT("arm"))  || Bone.Contains(TEXT("hand"))  ||
      Bone.Contains(TEXT("wrist")) || Bone.Contains(TEXT("elbow")) ||
      Bone.Contains(TEXT("shoulder")))
  {
    OpenFinisherWindow(EFinisherZone::Arm);
    return;
  }

  // Jambes
  if (Bone.Contains(TEXT("thigh")) || Bone.Contains(TEXT("calf"))  ||
      Bone.Contains(TEXT("leg"))   || Bone.Contains(TEXT("knee"))  ||
      Bone.Contains(TEXT("foot"))  || Bone.Contains(TEXT("ankle")))
  {
    OpenFinisherWindow(EFinisherZone::Leg);
  }
}

void AEnemyBase::OpenFinisherWindow(EFinisherZone Zone)
{
  ActiveFinisherZone = Zone;

  // Jouer une anim de stagger (HitReact) pour signaler la vulnérabilité
  if (HitReactAnim)
  {
    if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
    {
      AI->PlaySlotAnimationAsDynamicMontage(HitReactAnim, FName("DefaultSlot"), 0.1f, 0.2f, 1.f);
    }
  }

  // Ralentir l'ennemi pendant la fenêtre (feedback visuel du stagger)
  GetCharacterMovement()->MaxWalkSpeed *= 0.4f;

  // Timer de fermeture automatique
  GetWorldTimerManager().SetTimer(
    FinisherWindowTimer,
    this, &AEnemyBase::CloseFinisherWindow,
    FinisherWindowDuration, false
  );

  BP_OnFinisherWindowOpened(Zone);
}

void AEnemyBase::CloseFinisherWindow()
{
  ActiveFinisherZone = EFinisherZone::None;

  // Restaurer la vitesse
  if (!bIsStunned)
    GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->GetMaxSpeed();
}

void AEnemyBase::ApplyFinisher(float BaseDamage, AController* DamageInstigator, AActor* Causer)
{
  const EFinisherZone Zone = ActiveFinisherZone;

  // Fermer la fenêtre immédiatement
  GetWorldTimerManager().ClearTimer(FinisherWindowTimer);
  CloseFinisherWindow();

  // Dégâts amplifiés
  UGameplayStatics::ApplyDamage(this, BaseDamage * FinisherDamageMultiplier,
                                 DamageInstigator, Causer, nullptr);

  // Stun post-finisher
  ApplyStun(FinisherStunDuration);

  BP_OnFinisherExecuted(Zone);
}

void AEnemyBase::ApplyStun(float Duration)
{
  if (bIsDead || Duration <= 0.f) return;

  bIsStunned = true;

  // Sauvegarder la vitesse normale avant de l'annuler
  const float SavedSpeed = GetCharacterMovement()->MaxWalkSpeed;

  // Immobiliser
  GetCharacterMovement()->MaxWalkSpeed = 0.f;
  if (AAIController* AIC = Cast<AAIController>(GetController()))
    AIC->StopMovement();

  // Jouer la hit react comme animation de stun
  if (HitReactAnim)
  {
    if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
      AI->PlaySlotAnimationAsDynamicMontage(HitReactAnim, FName("DefaultSlot"), 0.1f, 0.2f, 1.f);
  }

  // Annuler et relancer le timer si un stun est déjà actif (refresh)
  GetWorldTimerManager().SetTimer(
    StunTimer,
    [this, SavedSpeed]()
    {
      bIsStunned = false;
      // Restaurer la vitesse seulement si pas encore en fenêtre de finisher
      if (ActiveFinisherZone == EFinisherZone::None)
        GetCharacterMovement()->MaxWalkSpeed = SavedSpeed;
    },
    Duration, false
  );
}
