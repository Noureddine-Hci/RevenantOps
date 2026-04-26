// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreEntry.h"
#include "LeaderboardWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

/**
 *  Local leaderboard (top 10 scores).
 *  Persists between sessions via SaveGame.
 */
UCLASS(abstract, Blueprintable)
class ULeaderboardWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual TSharedRef<SWidget> RebuildWidget() override;
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

  /** Persist a score entry without requiring a widget instance */
  UFUNCTION(BlueprintCallable, Category = "Leaderboard")
  static void SaveScoreStatic(UObject* WorldContext, int32 Score, int32 Kills, int32 BestCombo,
                               const FString& SlotName = TEXT("Leaderboard"), int32 MaxEntries = 10);

  /** Set the save slot before calling LoadScores */
  void SetSaveSlot(const FString& InSlotName) { SaveSlotName = InSlotName; }

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

  UPROPERTY(meta = (BindWidgetOptional))
  UButton* BackButton;

  UFUNCTION()
  void OnBackClicked();

  /** BP hook: called when scores are updated (rebuild UI list) */
  UFUNCTION(BlueprintImplementableEvent, Category = "Leaderboard",
            meta = (DisplayName = "On Scores Updated"))
  void BP_OnScoresUpdated();

private:
  void BuildDefaultUI();
  void RebuildScoreList();

  /** VBox for dynamically listing scores (auto-built UI only) */
  UPROPERTY()
  UVerticalBox* ScoreListBox = nullptr;
};
