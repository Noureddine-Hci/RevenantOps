// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ScoreEntry.generated.h"

/**
 *  Score entry for the local leaderboard.
 *  Shared between LeaderboardWidget and LeaderboardSaveGame.
 */
USTRUCT(BlueprintType)
struct FScoreEntry {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Score = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Kills = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 BestCombo = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Date;
};
