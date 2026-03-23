// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LeaderboardWidget.generated.h"

class UTextBlock;
class UVerticalBox;

/**
 *  Score entry for the local leaderboard.
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

/**
 *  Local leaderboard (top 10 scores).
 *  Persists between sessions via SaveGame.
 */
UCLASS(abstract, Blueprintable)
class ULeaderboardWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void NativeConstruct() override;

  /** Add a new score and refresh the display */
  UFUNCTION(BlueprintCallable, Category = "Leaderboard")
  void AddScore(int32 Score, int32 Kills, int32 BestCombo);

  /** Load scores from disk */
  UFUNCTION(BlueprintCallable, Category = "Leaderboard")
  void LoadScores();

  /** Save scores to disk */
  UFUNCTION(BlueprintCallable, Category = "Leaderboard")
  void SaveScores();

  /** Get the sorted score list */
  UFUNCTION(BlueprintCallable, Category = "Leaderboard")
  const TArray<FScoreEntry> &GetScores() const { return Scores; }

protected:
  /** Max entries in the leaderboard */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard",
            meta = (ClampMin = 5, ClampMax = 50))
  int32 MaxEntries = 10;

  /** Save slot name */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
  FString SaveSlotName = TEXT("Leaderboard");

  /** Sorted score entries (highest first) */
  UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
  TArray<FScoreEntry> Scores;

  /** BP hook: called when scores are updated (rebuild UI list) */
  UFUNCTION(BlueprintImplementableEvent, Category = "Leaderboard",
            meta = (DisplayName = "On Scores Updated"))
  void BP_OnScoresUpdated();
};
