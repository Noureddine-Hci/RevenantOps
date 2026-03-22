// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyWaveSpawner.generated.h"

class AEnemyBase;
class UBoxComponent;

/**
 *  Configuration for a single enemy in a wave
 */
USTRUCT(BlueprintType)
struct FWaveEnemyEntry {
  GENERATED_BODY()

  /** Enemy class to spawn */
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AEnemyBase> EnemyClass;

  /** Number of this enemy type in the wave */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 1, ClampMax = 50))
  int32 Count = 1;

  /** Delay before spawning this group (seconds from wave start) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 30))
  float SpawnDelay = 0.f;
};

/**
 *  Configuration for a wave of enemies
 */
USTRUCT(BlueprintType)
struct FEnemyWave {
  GENERATED_BODY()

  /** Enemies in this wave */
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FWaveEnemyEntry> Enemies;

  /** Delay before the next wave starts after this one is cleared */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 60))
  float DelayAfterWave = 3.f;
};

/**
 *  Delegate fired when a wave starts
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStarted,
                                              int32, WaveNumber,
                                              int32, TotalWaves);

/**
 *  Delegate fired when all waves are completed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);

/**
 *  Wave-based enemy spawner.
 *  Place in level with spawn points, configure waves in Blueprint.
 *  Can be triggered by volume overlap or manually.
 */
UCLASS(Blueprintable)
class AEnemyWaveSpawner : public AActor {
  GENERATED_BODY()

public:
  AEnemyWaveSpawner();

protected:
  virtual void BeginPlay() override;

  // ========== COMPONENTS ==========

  /** Trigger volume to start the encounter */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UBoxComponent *TriggerVolume;

  // ========== WAVE CONFIGURATION ==========

  /** List of waves to spawn */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Waves")
  TArray<FEnemyWave> Waves;

  /** If true, the encounter starts when the player enters the trigger volume */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
  bool bTriggerOnOverlap = true;

  /** If true, loops waves infinitely (survival mode) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
  bool bInfiniteMode = false;

  /** Spawn points (actors placed in the level) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
  TArray<AActor *> SpawnPoints;

  /** Maximum number of enemies alive at once */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config",
            meta = (ClampMin = 1, ClampMax = 50))
  int32 MaxAliveEnemies = 8;

  // ========== STATE ==========

  /** Current wave index */
  UPROPERTY(BlueprintReadOnly, Category = "Spawner|State")
  int32 CurrentWaveIndex = -1;

  /** Is the encounter active */
  UPROPERTY(BlueprintReadOnly, Category = "Spawner|State")
  bool bIsActive = false;

  /** Currently alive enemies from this spawner */
  UPROPERTY()
  TArray<AEnemyBase *> AliveEnemies;

  /** Total enemies killed across all waves */
  UPROPERTY(BlueprintReadOnly, Category = "Spawner|State")
  int32 TotalKilled = 0;

  /** Timer handles for delayed spawns */
  TArray<FTimerHandle> SpawnTimers;

  /** Timer for next wave delay */
  FTimerHandle NextWaveTimer;

public:
  // ========== EVENTS ==========

  UPROPERTY(BlueprintAssignable, Category = "Spawner|Events")
  FOnWaveStarted OnWaveStarted;

  UPROPERTY(BlueprintAssignable, Category = "Spawner|Events")
  FOnAllWavesCompleted OnAllWavesCompleted;

  // ========== PUBLIC API ==========

  /** Manually start the encounter */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  void StartEncounter();

  /** Get the current wave number (1-based for UI) */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  int32 GetCurrentWaveNumber() const { return CurrentWaveIndex + 1; }

  /** Get total number of waves */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  int32 GetTotalWaves() const { return Waves.Num(); }

  /** Get number of alive enemies */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  int32 GetAliveEnemyCount() const { return AliveEnemies.Num(); }

  /** Set the wave configuration at runtime */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  void SetWaves(const TArray<FEnemyWave>& NewWaves) { Waves = NewWaves; }

  /** Set the max alive enemies cap */
  UFUNCTION(BlueprintCallable, Category = "Spawner")
  void SetMaxAliveEnemies(int32 NewMax) { MaxAliveEnemies = FMath::Clamp(NewMax, 1, 50); }

protected:
  /** Starts the next wave */
  void StartNextWave();

  /** Spawns enemies from a wave entry */
  void SpawnWaveEntry(const FWaveEnemyEntry &Entry);

  /** Gets a random spawn point transform */
  FTransform GetRandomSpawnTransform() const;

  /** Called when an enemy dies */
  UFUNCTION()
  void OnEnemyDied(AEnemyBase *Enemy, AController *KilledBy);

  /** Trigger volume overlap */
  UFUNCTION()
  void OnTriggerOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                        UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult &SweepResult);

  // ========== BLUEPRINT HOOKS ==========

  UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events",
            meta = (DisplayName = "On Wave Started"))
  void BP_OnWaveStarted(int32 WaveNumber);

  UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events",
            meta = (DisplayName = "On All Waves Completed"))
  void BP_OnAllWavesCompleted();
};
