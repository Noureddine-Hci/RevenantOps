// Copyright RevenantOps. All Rights Reserved.

#include "GameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UGameOverWidget::NativeConstruct() {
  Super::NativeConstruct();

  if (ReplayButton) {
    ReplayButton->OnClicked.AddDynamic(this,
                                        &UGameOverWidget::OnReplayClicked);
  }

  if (QuitButton) {
    QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
  }
}

void UGameOverWidget::ShowResults(int32 FinalScore, int32 TotalKills,
                                   int32 BestCombo) {
  if (FinalScoreText) {
    FinalScoreText->SetText(
        FText::FromString(FString::Printf(TEXT("%d"), FinalScore)));
  }

  if (TotalKillsText) {
    TotalKillsText->SetText(
        FText::FromString(FString::Printf(TEXT("%d"), TotalKills)));
  }

  if (BestComboText) {
    BestComboText->SetText(
        FText::FromString(FString::Printf(TEXT("x%d"), BestCombo)));
  }

  BP_OnResultsShown(FinalScore, TotalKills, BestCombo);
}

void UGameOverWidget::OnReplayClicked() {
  // Restart current level
  UGameplayStatics::OpenLevel(
      GetWorld(), FName(*GetWorld()->GetName()), true);
}

void UGameOverWidget::OnQuitClicked() {
  UKismetSystemLibrary::QuitGame(
      GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
