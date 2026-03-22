// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnemyWaveSpawner.h"
#include "ZombieWaveConfig.generated.h"

class AEnemyWaveSpawner;

/**
 *  Blueprint function library providing default zombie wave configurations.
 *  Returns 10 progressive waves with escalating difficulty.
 */
UCLASS()
class UZombieWaveConfig : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

public:
  /** Returns 10 pre-configured zombie waves with progressive difficulty */
  UFUNCTION(BlueprintCallable, Category = "Zombie|Waves")
  static TArray<FEnemyWave> GetDefaultZombieWaves();

  /** Configures an EnemyWaveSpawner with the default 10 zombie waves and sets MaxAliveEnemies to 15 */
  UFUNCTION(BlueprintCallable, Category = "Zombie|Waves")
  static void ConfigureSpawnerWithDefaultWaves(AEnemyWaveSpawner* Spawner);
};
