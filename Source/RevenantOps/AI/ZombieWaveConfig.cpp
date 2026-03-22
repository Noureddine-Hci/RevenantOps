// Copyright RevenantOps. All Rights Reserved.

#include "ZombieWaveConfig.h"
#include "EnemyWaveSpawner.h"
#include "ZombieSlow.h"
#include "ZombieRunner.h"
#include "ZombieTank.h"
#include "ZombieExploder.h"
#include "ZombieSpitter.h"

TArray<FEnemyWave> UZombieWaveConfig::GetDefaultZombieWaves() {
  TArray<FEnemyWave> Waves;

  // Helper to create a wave enemy entry
  auto MakeEntry = [](TSubclassOf<AEnemyBase> Class, int32 Count,
                      float Delay = 0.f) -> FWaveEnemyEntry {
    FWaveEnemyEntry Entry;
    Entry.EnemyClass = Class;
    Entry.Count = Count;
    Entry.SpawnDelay = Delay;
    return Entry;
  };

  // --- Wave 1: 5 ZombieSlow ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 5));
    Wave.DelayAfterWave = 5.0f;
    Waves.Add(Wave);
  }

  // --- Wave 2: 8 ZombieSlow ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 8));
    Wave.DelayAfterWave = 5.0f;
    Waves.Add(Wave);
  }

  // --- Wave 3: 10 ZombieSlow (5 immediate + 5 delayed) ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 5));
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 5, 3.0f));
    Wave.DelayAfterWave = 4.0f;
    Waves.Add(Wave);
  }

  // --- Wave 4: 6 ZombieSlow + 3 ZombieRunner ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 6));
    Wave.Enemies.Add(MakeEntry(AZombieRunner::StaticClass(), 3, 2.0f));
    Wave.DelayAfterWave = 4.0f;
    Waves.Add(Wave);
  }

  // --- Wave 5: 8 ZombieSlow + 5 ZombieRunner ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 8));
    Wave.Enemies.Add(MakeEntry(AZombieRunner::StaticClass(), 5, 1.0f));
    Wave.DelayAfterWave = 4.0f;
    Waves.Add(Wave);
  }

  // --- Wave 6: 6 ZombieSlow + 4 ZombieRunner + 1 ZombieTank ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 6));
    Wave.Enemies.Add(MakeEntry(AZombieRunner::StaticClass(), 4));
    Wave.Enemies.Add(MakeEntry(AZombieTank::StaticClass(), 1, 3.0f));
    Wave.DelayAfterWave = 5.0f;
    Waves.Add(Wave);
  }

  // --- Wave 7: 8 ZombieSlow + 5 ZombieRunner + 2 ZombieTank ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 8));
    Wave.Enemies.Add(MakeEntry(AZombieRunner::StaticClass(), 5));
    Wave.Enemies.Add(MakeEntry(AZombieTank::StaticClass(), 2, 4.0f));
    Wave.DelayAfterWave = 5.0f;
    Waves.Add(Wave);
  }

  // --- Wave 8: 6 ZombieSlow + 4 ZombieRunner + 2 ZombieTank + 2 ZombieSpitter ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 6));
    Wave.Enemies.Add(MakeEntry(AZombieRunner::StaticClass(), 4));
    Wave.Enemies.Add(MakeEntry(AZombieTank::StaticClass(), 2));
    Wave.Enemies.Add(MakeEntry(AZombieSpitter::StaticClass(), 2, 3.0f));
    Wave.DelayAfterWave = 5.0f;
    Waves.Add(Wave);
  }

  // --- Wave 9: 8 ZombieSlow + 5 ZombieRunner + 2 ZombieTank + 3 ZombieSpitter + 2 ZombieExploder ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 8));
    Wave.Enemies.Add(MakeEntry(AZombieRunner::StaticClass(), 5));
    Wave.Enemies.Add(MakeEntry(AZombieTank::StaticClass(), 2));
    Wave.Enemies.Add(MakeEntry(AZombieSpitter::StaticClass(), 3));
    Wave.Enemies.Add(MakeEntry(AZombieExploder::StaticClass(), 2, 5.0f));
    Wave.DelayAfterWave = 5.0f;
    Waves.Add(Wave);
  }

  // --- Wave 10: 10 ZombieSlow + 6 ZombieRunner + 3 ZombieTank + 3 ZombieSpitter + 3 ZombieExploder ---
  {
    FEnemyWave Wave;
    Wave.Enemies.Add(MakeEntry(AZombieSlow::StaticClass(), 10));
    Wave.Enemies.Add(MakeEntry(AZombieRunner::StaticClass(), 6));
    Wave.Enemies.Add(MakeEntry(AZombieTank::StaticClass(), 3));
    Wave.Enemies.Add(MakeEntry(AZombieSpitter::StaticClass(), 3));
    Wave.Enemies.Add(MakeEntry(AZombieExploder::StaticClass(), 3));
    Wave.DelayAfterWave = 0.0f; // Final wave
    Waves.Add(Wave);
  }

  return Waves;
}

void UZombieWaveConfig::ConfigureSpawnerWithDefaultWaves(
    AEnemyWaveSpawner* Spawner) {
  if (!Spawner) {
    return;
  }

  Spawner->SetWaves(GetDefaultZombieWaves());
  Spawner->SetMaxAliveEnemies(15);
}
