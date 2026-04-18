// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButton;
class UTextBlock;

/**
 *  Game over / end of match screen.
 *  Shows final score, kill count, best combo, and replay/quit options.
 */
UCLASS(abstract, Blueprintable)
class UGameOverWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;

  /** Populates the screen with match results.
   *  @param bVictory  true = toutes les vagues complétées, false = mort */
  UFUNCTION(BlueprintCallable, Category = "UI|GameOver")
  void ShowResults(int32 FinalScore, int32 TotalKills, int32 BestCombo, bool bVictory = false);

protected:
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *TitleText;

  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *FinalScoreText;

  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *TotalKillsText;

  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *BestComboText;

  UPROPERTY(meta = (BindWidgetOptional))
  UButton *ReplayButton;

  UPROPERTY(meta = (BindWidgetOptional))
  UButton *LeaderboardButton;

  UPROPERTY(meta = (BindWidgetOptional))
  UButton *QuitButton;

  UFUNCTION()
  void OnReplayClicked();

  UFUNCTION()
  void OnLeaderboardClicked();

  UFUNCTION()
  void OnQuitClicked();

  /** BP hook for animations/effects */
  UFUNCTION(BlueprintImplementableEvent, Category = "UI|GameOver",
            meta = (DisplayName = "On Results Shown"))
  void BP_OnResultsShown(int32 Score, int32 Kills, int32 Combo);

private:
  void BuildDefaultUI();
};
