// Copyright RevenantOps. All Rights Reserved.

#include "MercenairesGameState.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyBase.h"

AMercenairesGameState::AMercenairesGameState() {
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bStartWithTickEnabled = true;
}

void AMercenairesGameState::Tick(float DeltaSeconds) {
  Super::Tick(DeltaSeconds);

  if (!bMatchActive) {
    return;
  }

  // Countdown timer
  TimeRemaining -= DeltaSeconds;

  if (TimeRemaining <= 0.f) {
    TimeRemaining = 0.f;
    OnTimerChanged.Broadcast(TimeRemaining);
    EndMatch();
    return;
  }

  OnTimerChanged.Broadcast(TimeRemaining);

  // Combo decay
  if (ComboMultiplier > 1) {
    ComboTimeRemaining -= DeltaSeconds;
    if (ComboTimeRemaining <= 0.f) {
      ResetCombo();
    } else {
      OnComboChanged.Broadcast(ComboMultiplier, ComboTimeRemaining);
    }
  }
}

void AMercenairesGameState::StartMatch() {
  TimeRemaining = MatchDuration;
  CurrentScore = 0;
  ComboMultiplier = 1;
  ComboTimeRemaining = 0.f;
  TotalKills = 0;
  BestCombo = 1;
  bMatchActive = true;

  // Start background music
  if (BackgroundMusic) {
    MusicComponent = UGameplayStatics::SpawnSound2D(this, BackgroundMusic, MusicVolume, 1.f, 0.f, nullptr, false, false);
  }

  OnMatchStateChanged.Broadcast(true);
  OnTimerChanged.Broadcast(TimeRemaining);
  OnScoreChanged.Broadcast(0, 0);
  OnComboChanged.Broadcast(1, 0.f);
}

void AMercenairesGameState::EndMatch() {
  bMatchActive = false;

  // Stop background music
  if (MusicComponent) {
    MusicComponent->FadeOut(2.f, 0.f);
  }

  OnMatchStateChanged.Broadcast(false);
}

void AMercenairesGameState::AddBonusTime(float Seconds) {
  if (!bMatchActive) {
    return;
  }

  TimeRemaining += Seconds;
  OnTimerChanged.Broadcast(TimeRemaining);
}

void AMercenairesGameState::RegisterKill(AEnemyBase *KilledEnemy) {
  if (!bMatchActive || !KilledEnemy) {
    return;
  }

  TotalKills++;

  // Determine base points for this enemy type
  const int32 BasePoints = GetPointsForEnemy(KilledEnemy);

  // Apply combo multiplier
  const int32 PointsEarned = BasePoints * ComboMultiplier;
  CurrentScore += PointsEarned;

  // Increase combo
  ComboMultiplier = FMath::Min(ComboMultiplier + 1, MaxComboMultiplier);
  ComboTimeRemaining = ComboWindowDuration;

  // Track best combo
  if (ComboMultiplier > BestCombo) {
    BestCombo = ComboMultiplier;
  }

  // Broadcast updates
  OnScoreChanged.Broadcast(CurrentScore, PointsEarned);
  OnComboChanged.Broadcast(ComboMultiplier, ComboTimeRemaining);
}

int32 AMercenairesGameState::GetPointsForEnemy(AEnemyBase *Enemy) const {
  if (!Enemy) {
    return DefaultKillPoints;
  }

  return Enemy->GetKillPoints();
}

void AMercenairesGameState::ResetCombo() {
  ComboMultiplier = 1;
  ComboTimeRemaining = 0.f;
  OnComboChanged.Broadcast(1, 0.f);
}
