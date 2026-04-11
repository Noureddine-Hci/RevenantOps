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

  // If no waves were configured in the editor, build default waves at runtime.
  if (Waves.IsEmpty()) {
    SetupDefaultWaves();
  }
}

void AEnemyWaveSpawner::SetupDefaultWaves()
{
  static const TCHAR* ZombiePaths[] = {
    TEXT("/Game/Mercenaires/Zombies/BP_ZombieSlow.BP_ZombieSlow_C"),
    TEXT("/Game/Mercenaires/Zombies/BP_ZombieRunner.BP_ZombieRunner_C"),
    TEXT("/Game/Mercenaires/Zombies/BP_ZombieTank.BP_ZombieTank_C"),
    TEXT("/Game/Mercenaires/Zombies/BP_ZombieSpitter.BP_ZombieSpitter_C"),
  };

  TSubclassOf<AEnemyBase> SlowClass, RunnerClass, TankClass;
  for (const TCHAR* Path : ZombiePaths)
  {
    if (UClass* C = LoadClass<AEnemyBase>(nullptr, Path))
    {
      FString PathStr(Path);
      if (!SlowClass   && PathStr.Contains(TEXT("Slow")))   SlowClass   = C;
      if (!RunnerClass && PathStr.Contains(TEXT("Runner"))) RunnerClass = C;
      if (!TankClass   && PathStr.Contains(TEXT("Tank")))   TankClass   = C;
      if (!SlowClass   && PathStr.Contains(TEXT("Spitter"))) SlowClass  = C;
    }
  }

  // Fallback: use whatever loaded first if specific types not found
  for (const TCHAR* Path : ZombiePaths)
  {
    if (!SlowClass) SlowClass = LoadClass<AEnemyBase>(nullptr, Path);
    if (SlowClass) break;
  }

  if (!SlowClass)
  {
    UE_LOG(LogTemp, Warning, TEXT("EnemyWaveSpawner: no zombie class found, cannot build default waves."));
    return;
  }

  if (!RunnerClass) RunnerClass = SlowClass;
  if (!TankClass)   TankClass   = SlowClass;

  // Wave 1: 5 slow zombies
  {
    FEnemyWave W;
    W.DelayAfterWave = 3.f;
    FWaveEnemyEntry E; E.EnemyClass = SlowClass; E.Count = 5; E.SpawnDelay = 0.f;
    W.Enemies.Add(E);
    Waves.Add(W);
  }
  // Wave 2: 4 slow + 3 runners (runners delayed 5s)
  {
    FEnemyWave W;
    W.DelayAfterWave = 3.f;
    FWaveEnemyEntry E1; E1.EnemyClass = SlowClass;   E1.Count = 4; E1.SpawnDelay = 0.f;
    FWaveEnemyEntry E2; E2.EnemyClass = RunnerClass; E2.Count = 3; E2.SpawnDelay = 5.f;
    W.Enemies.Add(E1); W.Enemies.Add(E2);
    Waves.Add(W);
  }
  // Wave 3: 4 runners + 2 tanks (tanks delayed 8s)
  {
    FEnemyWave W;
    W.DelayAfterWave = 5.f;
    FWaveEnemyEntry E1; E1.EnemyClass = RunnerClass; E1.Count = 4; E1.SpawnDelay = 0.f;
    FWaveEnemyEntry E2; E2.EnemyClass = TankClass;   E2.Count = 2; E2.SpawnDelay = 8.f;
    W.Enemies.Add(E1); W.Enemies.Add(E2);
    Waves.Add(W);
  }

  UE_LOG(LogTemp, Log, TEXT("EnemyWaveSpawner: built %d default waves (Slow/Runner/Tank)."), Waves.Num());
}

// =============================================================================
// ENCOUNTER MANAGEMENT
// =============================================================================

void AEnemyWaveSpawner::StartEncounter() {
  UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] StartEncounter called. bIsActive=%d, Waves=%d"), bIsActive, Waves.Num());
  if (bIsActive) {
    UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] Already active, ignoring."));
    return;
  }

  bIsActive = true;
  CurrentWaveIndex = -1;
  TotalKilled = 0;

  // Clear any leftover timers from previous encounter
  for (FTimerHandle &Handle : SpawnTimers) {
    GetWorld()->GetTimerManager().ClearTimer(Handle);
  }
  SpawnTimers.Empty();
  GetWorld()->GetTimerManager().ClearTimer(NextWaveTimer);
  AliveEnemies.Empty();

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
    UE_LOG(LogTemp, Error, TEXT("[WaveSpawner] SpawnWaveEntry: EnemyClass is NULL!"));
    return;
  }

  UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] SpawnWaveEntry: class=%s count=%d alive=%d/%d"),
    *Entry.EnemyClass->GetName(), Entry.Count, AliveEnemies.Num(), MaxAliveEnemies);

  for (int32 i = 0; i < Entry.Count; ++i) {
    // Respect max alive limit
    if (AliveEnemies.Num() >= MaxAliveEnemies) {
      break;
    }

    const FTransform SpawnTransform = GetRandomSpawnTransform();
    const FVector SL = SpawnTransform.GetLocation();
    UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] Spawning at (%.0f,%.0f,%.0f)"), SL.X, SL.Y, SL.Z);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyBase *NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(
        Entry.EnemyClass, SpawnTransform, SpawnParams);

    if (NewEnemy) {
      UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] Spawned OK: %s"), *NewEnemy->GetName());
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
  // Try configured SpawnPoints first — but only if they're within 2000u of the player
  APawn *Player = UGameplayStatics::GetPlayerPawn(this, 0);
  const FVector PlayerLoc = Player ? Player->GetActorLocation() : GetActorLocation();

  if (SpawnPoints.Num() > 0) {
    TArray<int32> ValidIndices;
    for (int32 i = 0; i < SpawnPoints.Num(); ++i) {
      if (SpawnPoints[i]) {
        ValidIndices.Add(i);
      }
    }

    if (ValidIndices.Num() > 0) {
      const int32 Pick = ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)];
      return SpawnPoints[Pick]->GetActorTransform();
    }
  }

  // Spawn directly in front/sides of the player at 300-400u — guaranteed visible
  if (Player)
  {
    const FVector Fwd   = Player->GetActorForwardVector();
    const FVector Right = Player->GetActorRightVector();
    const float   Fwd2D   = FMath::FRandRange(250.f, 400.f);
    const float   Side2D  = FMath::FRandRange(-200.f, 200.f);
    const FVector SpawnLoc = PlayerLoc + Fwd * Fwd2D + Right * Side2D;
    return FTransform(FRotator::ZeroRotator, FVector(SpawnLoc.X, SpawnLoc.Y, PlayerLoc.Z));
  }

  // Last resort: directly at spawner location
  return FTransform(FRotator::ZeroRotator, GetActorLocation());
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
