// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/MenuWidgetBase.h"
#include "GameOverWidget.generated.h"

class UButton;
class UTextBlock;

/**
 *  Game over / end of match screen.
 *  Shows final score, kill count, best combo, and replay/quit options.
 */
UCLASS(abstract, Blueprintable)
class UGameOverWidget : public UMenuWidgetBase {
  GENERATED_BODY()

public:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;

  /** Populates the screen with match results.
   *  @param bVictory  true = toutes les vagues complétées, false = mort */
  UFUNCTION(BlueprintCallable, Category = "UI|GameOver")
  void ShowResults(int32 FinalScore, int32 TotalKills, int32 BestCombo, bool bVictory = false);

  /**
   *  Calcule le rang S/A/B/C/D selon le score et le combo.
   *  S ≥ 5000 | A ≥ 3000 | B ≥ 1500 | C ≥ 500 | D < 500
   */
  static FString ComputeRank(int32 Score, int32 BestCombo);

protected:
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *TitleText;

  /** Grande lettre du rang (S / A / B / C / D) */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *RankText;

  /** Sous-label "EVALUATION" au-dessus de la lettre */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *RankLabelText;

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
