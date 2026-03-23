// Copyright RevenantOps. All Rights Reserved.

#include "LeaderboardWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"

// Simple SaveGame subclass for leaderboard persistence
#include "LeaderboardSaveGame.h"

void ULeaderboardWidget::NativeConstruct() {
  Super::NativeConstruct();
  LoadScores();
}

void ULeaderboardWidget::AddScore(int32 Score, int32 Kills, int32 BestCombo) {
  FScoreEntry Entry;
  Entry.Score = Score;
  Entry.Kills = Kills;
  Entry.BestCombo = BestCombo;
  Entry.Date = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M"));

  Scores.Add(Entry);

  // Sort descending by score
  Scores.Sort([](const FScoreEntry &A, const FScoreEntry &B) {
    return A.Score > B.Score;
  });

  // Trim to max entries
  if (Scores.Num() > MaxEntries) {
    Scores.SetNum(MaxEntries);
  }

  SaveScores();
  BP_OnScoresUpdated();
}

void ULeaderboardWidget::LoadScores() {
  ULeaderboardSaveGame *SaveGame = Cast<ULeaderboardSaveGame>(
      UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

  if (SaveGame) {
    Scores = SaveGame->Scores;
  } else {
    Scores.Empty();
  }

  BP_OnScoresUpdated();
}

void ULeaderboardWidget::SaveScores() {
  ULeaderboardSaveGame *SaveGame =
      Cast<ULeaderboardSaveGame>(
          UGameplayStatics::CreateSaveGameObject(
              ULeaderboardSaveGame::StaticClass()));

  if (SaveGame) {
    SaveGame->Scores = Scores;
    UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
  }
}
