// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LeaderboardWidget.h"
#include "LeaderboardSaveGame.generated.h"

/**
 *  SaveGame class for persisting leaderboard scores.
 */
UCLASS()
class ULeaderboardSaveGame : public USaveGame {
  GENERATED_BODY()

public:
  UPROPERTY()
  TArray<FScoreEntry> Scores;
};
