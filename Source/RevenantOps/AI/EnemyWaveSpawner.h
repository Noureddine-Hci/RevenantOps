// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyWaveSpawner.generated.h"

class AEnemyBase;
class UBoxComponent;

/**
 *  Entrée de la pool d'ennemis — classe + poids de sélection aléatoire.
 *  Plus le poids est élevé, plus l'ennemi apparaît fréquemment.
 */
USTRUCT(BlueprintType)
struct FEnemyPoolEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
    TSubclassOf<AEnemyBase> EnemyClass;

    /**
     *  Poids de sélection (1 = normal, 2 = deux fois plus probable).
     *  Ce poids est multiplié par le ScaleAtMaxDifficulty quand la difficulté est max.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool",
              meta = (ClampMin = 0.1f, ClampMax = 10.f))
    float Weight = 1.f;

    /**
     *  Seuil de kills à partir duquel cet ennemi peut apparaître (0 = dès le début).
     *  Exemple : Tank = 30 kills minimum.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool",
              meta = (ClampMin = 0))
    int32 UnlockAtKills = 0;
};

// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillCountChanged, int32, TotalKills);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKillCapReached);

/**
 *  Spawner style RE5 Mercenaires.
 *
 *  - Pool d'ennemis avec poids et seuils d'apparition
 *  - Cap de kills configurable (défaut 150)
 *  - Max ennemis simultanés configurable (défaut 20)
 *  - Dès qu'un ennemi meurt → un nouveau spawne immédiatement
 *  - Difficulté croissante : spawn delay diminue avec les kills
 */
UCLASS(Blueprintable)
class AEnemyWaveSpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemyWaveSpawner();

protected:
    virtual void BeginPlay() override;

    // ── COMPOSANTS ────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerVolume;

    // ── CONFIGURATION ─────────────────────────────────────────────────────

    /** Pool d'ennemis disponibles avec leurs poids */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
    TArray<FEnemyPoolEntry> EnemyPool;

    /** Points de spawn dans le level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
    TArray<AActor*> SpawnPoints;

    /** Nombre total de kills pour finir le match (RE5 = 150) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config",
              meta = (ClampMin = 10, ClampMax = 500))
    int32 KillCap = 150;

    /** Nombre max d'ennemis en vie simultanément */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config",
              meta = (ClampMin = 1, ClampMax = 50))
    int32 MaxAliveEnemies = 20;

    /** Délai initial entre chaque spawn (secondes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config",
              meta = (ClampMin = 0.1f, ClampMax = 5.f))
    float SpawnDelayInitial = 1.5f;

    /** Délai minimal entre spawns (à difficulté max) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config",
              meta = (ClampMin = 0.1f, ClampMax = 3.f))
    float SpawnDelayMinimum = 0.3f;

    /** Si true, démarre automatiquement au BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
    bool bAutoStart = true;

    /** Si true, démarre quand le joueur entre dans le TriggerVolume */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Config")
    bool bTriggerOnOverlap = false;

    // ── ÉTAT ──────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Spawner|State")
    int32 TotalKilled = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Spawner|State")
    bool bIsActive = false;

    UPROPERTY()
    TArray<AEnemyBase*> AliveEnemies;

    FTimerHandle SpawnLoopTimer;

    // ── LOGIQUE INTERNE ───────────────────────────────────────────────────

    void SpawnOneEnemy();
    TSubclassOf<AEnemyBase> PickEnemyClass() const;
    FTransform GetRandomSpawnTransform() const;
    float GetCurrentSpawnDelay() const;
    void SetupDefaultPool();

    UFUNCTION()
    void OnEnemyDied(AEnemyBase* Enemy, AController* KilledBy);

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                          bool bFromSweep, const FHitResult& SweepResult);

    // ── HOOKS BP ──────────────────────────────────────────────────────────


public:
    // ── API PUBLIQUE ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Spawner|Events")
    FOnKillCountChanged OnKillCountChanged;

    UPROPERTY(BlueprintAssignable, Category = "Spawner|Events")
    FOnKillCapReached OnKillCapReached;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void StartEncounter();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void StopEncounter();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    int32 GetTotalKilled() const { return TotalKilled; }

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    int32 GetKillCap() const { return KillCap; }

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    int32 GetAliveEnemyCount() const { return AliveEnemies.Num(); }

    // Compatibilité avec l'ancien système (HUD etc.)
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    int32 GetCurrentWaveNumber() const { return FMath::Min(TotalKilled / 30 + 1, 5); }

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    int32 GetTotalWaves() const { return KillCap / 30; }
};
