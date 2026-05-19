// Copyright RevenantOps. All Rights Reserved.

#include "MercenairesGameState.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyBase.h"
#include "RevenantOpsPlayerController.h"
#include "UI/RevenantOpsHUD.h"

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

  // Switch to urgent music when time is low
  if (!bLowTimeMusicTriggered && LowTimeMusic && TimeRemaining <= LowTimeThreshold)
  {
    bLowTimeMusicTriggered = true;
    if (MusicComponent)
      MusicComponent->FadeOut(1.f, 0.f);
    MusicComponent = UGameplayStatics::SpawnSound2D(
        this, LowTimeMusic, 0.f, 1.f, 0.f, nullptr, false, false);
    if (MusicComponent)
      MusicComponent->FadeIn(1.f, MusicVolume);
  }

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
  bLowTimeMusicTriggered = false;

  // Start background music with fade-in
  if (BackgroundMusic)
  {
    MusicComponent = UGameplayStatics::SpawnSound2D(
        this, BackgroundMusic, 0.f, 1.f, 0.f, nullptr, false, false);
    if (MusicComponent)
    {
      if (MusicFadeInDuration > 0.f)
        MusicComponent->FadeIn(MusicFadeInDuration, MusicVolume);
      else
        MusicComponent->SetVolumeMultiplier(MusicVolume);
    }
  }

  OnMatchStateChanged.Broadcast(true);
  OnTimerChanged.Broadcast(TimeRemaining);
  OnScoreChanged.Broadcast(0, 0);
  OnComboChanged.Broadcast(1, 0.f);

  // Afficher le message "C'est Parti !" sur le HUD du joueur local
  if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
  {
    if (ARevenantOpsPlayerController* ROPC = Cast<ARevenantOpsPlayerController>(PC))
    {
      if (URevenantOpsHUD* HUD = ROPC->GetHUDWidget())
      {
        HUD->ShowMatchStartMessage(NSLOCTEXT("Mercenaires", "MatchStart", "C'est Parti !"));
      }
    }
  }
}

void AMercenairesGameState::EndMatch() {
  bMatchActive = false;
  bLowTimeMusicTriggered = false;

  if (MusicComponent)
    MusicComponent->FadeOut(2.f, 0.f);

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
