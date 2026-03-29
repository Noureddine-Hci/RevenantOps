// Copyright RevenantOps. All Rights Reserved.

#include "EnemyWaveSpawner.h"
#include "Components/BoxComponent.h"
#include "EnemyBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AEnemyWaveSpawner::AEnemyWaveSpawner() {
  PrimaryActorTick.bCanEverTick = false;

  // Create trigger volume
  TriggerVolume =
      CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
  RootComponent = TriggerVolume;
  TriggerVolume->SetBoxExtent(FVector(500.f, 500.f, 200.f));
  TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AEnemyWaveSpawner::BeginPlay() {
  Super::BeginPlay();

  if (bTriggerOnOverlap) {
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(
        this, &AEnemyWaveSpawner::OnTriggerOverlap);
  }
}

// =============================================================================
// ENCOUNTER MANAGEMENT
// =============================================================================

void AEnemyWaveSpawner::StartEncounter() {
  if (bIsActive) {
    return;
  }

  bIsActive = true;
  CurrentWaveIndex = -1;
  TotalKilled = 0;

  StartNextWave();
}

void AEnemyWaveSpawner::StartNextWave() {
  ++CurrentWaveIndex;

  // Check if all waves are done
  if (!bInfiniteMode && CurrentWaveIndex >= Waves.Num()) {
    bIsActive = false;
    OnAllWavesCompleted.Broadcast();
    BP_OnAllWavesCompleted();
    return;
  }

  // In infinite mode, loop back
  const int32 WaveIdx =
      bInfiniteMode ? CurrentWaveIndex % Waves.Num() : CurrentWaveIndex;
  const FEnemyWave &CurrentWave = Waves[WaveIdx];

  // Broadcast wave start
  OnWaveStarted.Broadcast(CurrentWaveIndex + 1, Waves.Num());
  BP_OnWaveStarted(CurrentWaveIndex + 1);

  // Spawn each enemy group (with optional delay)
  for (const FWaveEnemyEntry &Entry : CurrentWave.Enemies) {
    if (Entry.SpawnDelay > 0.f) {
      FTimerHandle TimerHandle;
      FWaveEnemyEntry EntryCopy = Entry;
      GetWorld()->GetTimerManager().SetTimer(
          TimerHandle,
          [this, EntryCopy]() { SpawnWaveEntry(EntryCopy); },
          Entry.SpawnDelay, false);
      SpawnTimers.Add(TimerHandle);
    } else {
      SpawnWaveEntry(Entry);
    }
  }
}

void AEnemyWaveSpawner::SpawnWaveEntry(const FWaveEnemyEntry &Entry) {
  if (!Entry.EnemyClass) {
    return;
  }

  for (int32 i = 0; i < Entry.Count; ++i) {
    // Respect max alive limit
    if (AliveEnemies.Num() >= MaxAliveEnemies) {
      break;
    }

    const FTransform SpawnTransform = GetRandomSpawnTransform();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyBase *NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(
        Entry.EnemyClass, SpawnTransform, SpawnParams);

    if (NewEnemy) {
      AliveEnemies.Add(NewEnemy);

      // Subscribe to death event
      NewEnemy->OnEnemyDied.AddDynamic(this,
                                        &AEnemyWaveSpawner::OnEnemyDied);

      // Immediately alert spawned enemies to the player
      if (APawn *Player = UGameplayStatics::GetPlayerPawn(this, 0)) {
        NewEnemy->AlertToLocation(Player->GetActorLocation());
      }
    }
  }
}

FTransform AEnemyWaveSpawner::GetRandomSpawnTransform() const {
  if (SpawnPoints.Num() > 0) {
    const int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
    if (SpawnPoints[RandomIndex]) {
      return SpawnPoints[RandomIndex]->GetActorTransform();
    }
  }

  // Fallback: spawn at spawner location with random offset
  FVector SpawnLocation = GetActorLocation();
  SpawnLocation.X += FMath::FRandRange(-300.f, 300.f);
  SpawnLocation.Y += FMath::FRandRange(-300.f, 300.f);

  return FTransform(GetActorRotation(), SpawnLocation);
}

// =============================================================================
// EVENTS
// =============================================================================

void AEnemyWaveSpawner::OnEnemyDied(AEnemyBase *Enemy,
                                      AController *KilledBy) {
  AliveEnemies.Remove(Enemy);
  ++TotalKilled;

  // Check if wave is cleared
  if (AliveEnemies.Num() <= 0 && bIsActive) {
    // All enemies in this wave are dead
    const int32 WaveIdx =
        bInfiniteMode ? CurrentWaveIndex % Waves.Num() : CurrentWaveIndex;

    if (Waves.IsValidIndex(WaveIdx)) {
      const float Delay = Waves[WaveIdx].DelayAfterWave;
      GetWorld()->GetTimerManager().SetTimer(
          NextWaveTimer, this, &AEnemyWaveSpawner::StartNextWave, Delay,
          false);
    }
  }
}

void AEnemyWaveSpawner::OnTriggerOverlap(
    UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  // Only trigger for the player
  if (OtherActor && OtherActor->ActorHasTag(FName("Player"))) {
    StartEncounter();

    // Disable trigger after activation
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }
}
