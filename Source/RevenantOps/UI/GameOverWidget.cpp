// Copyright RevenantOps. All Rights Reserved.

#include "GameOverWidget.h"
#include "Styling/CoreStyle.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RevenantOpsPlayerController.h"

TSharedRef<SWidget> UGameOverWidget::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    BuildDefaultUI();
  }
  return Super::RebuildWidget();
}

void UGameOverWidget::NativeConstruct() {
  Super::NativeConstruct();

  if (ReplayButton) {
    ReplayButton->OnClicked.AddDynamic(this,
                                        &UGameOverWidget::OnReplayClicked);
  }

  if (LeaderboardButton) {
    LeaderboardButton->OnClicked.AddDynamic(
        this, &UGameOverWidget::OnLeaderboardClicked);
  }

  if (QuitButton) {
    QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
  }
}

void UGameOverWidget::BuildDefaultUI() {
  if (!WidgetTree) return;

  if (!WidgetTree->RootWidget) {
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
  }

  UCanvasPanel* Canvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
  if (!Canvas) return;

  // Full-screen dark background
  UImage* Background = WidgetTree->ConstructWidget<UImage>();
  Background->SetColorAndOpacity(FLinearColor(0.02f, 0.02f, 0.05f, 0.95f));
  UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(Background);
  BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
  BgSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 0.f));
  BgSlot->SetZOrder(0);

  UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
  UCanvasPanelSlot* VBoxSlot = Canvas->AddChildToCanvas(VBox);
  VBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
  VBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
  VBoxSlot->SetAutoSize(true);

  FSlateFontInfo LargeFont = FCoreStyle::GetDefaultFontStyle("Bold", 36);
  FSlateFontInfo MedFont   = FCoreStyle::GetDefaultFontStyle("Regular", 22);
  FSlateFontInfo BtnFont   = FCoreStyle::GetDefaultFontStyle("Regular", 20);

  // Title (mis à jour selon victoire/défaite dans ShowResults)
  TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
  TitleText->SetText(FText::FromString(TEXT("VOUS ÊTES MORT")));
  TitleText->SetFont(LargeFont);
  TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.2f, 0.2f)));
  TitleText->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(TitleText);
  TitleSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  TitleSlot->SetPadding(FMargin(0, 0, 0, 30));

  // Score
  FinalScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FinalScoreText"));
  FinalScoreText->SetText(FText::FromString(TEXT("Score: 0")));
  FinalScoreText->SetFont(MedFont);
  FinalScoreText->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* ScoreSlot = VBox->AddChildToVerticalBox(FinalScoreText);
  ScoreSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  ScoreSlot->SetPadding(FMargin(0, 0, 0, 10));

  // Kills
  TotalKillsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TotalKillsText"));
  TotalKillsText->SetText(FText::FromString(TEXT("Kills: 0")));
  TotalKillsText->SetFont(MedFont);
  TotalKillsText->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* KillsSlot = VBox->AddChildToVerticalBox(TotalKillsText);
  KillsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  KillsSlot->SetPadding(FMargin(0, 0, 0, 10));

  // Best Combo
  BestComboText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BestComboText"));
  BestComboText->SetText(FText::FromString(TEXT("Best Combo: x0")));
  BestComboText->SetFont(MedFont);
  BestComboText->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* ComboSlot = VBox->AddChildToVerticalBox(BestComboText);
  ComboSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  ComboSlot->SetPadding(FMargin(0, 0, 0, 30));

  // Replay Button
  ReplayButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ReplayButton"));
  UTextBlock* ReplayText = WidgetTree->ConstructWidget<UTextBlock>();
  ReplayText->SetText(FText::FromString(TEXT("REJOUER")));
  ReplayText->SetFont(BtnFont);
  ReplayText->SetJustification(ETextJustify::Center);
  ReplayButton->AddChild(ReplayText);
  UVerticalBoxSlot* ReplaySlot = VBox->AddChildToVerticalBox(ReplayButton);
  ReplaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  ReplaySlot->SetPadding(FMargin(0, 0, 0, 10));

  // Leaderboard Button
  LeaderboardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LeaderboardButton"));
  UTextBlock* LBText = WidgetTree->ConstructWidget<UTextBlock>();
  LBText->SetText(FText::FromString(TEXT("LEADERBOARD")));
  LBText->SetFont(BtnFont);
  LBText->SetJustification(ETextJustify::Center);
  LeaderboardButton->AddChild(LBText);
  UVerticalBoxSlot* LBSlot = VBox->AddChildToVerticalBox(LeaderboardButton);
  LBSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  LBSlot->SetPadding(FMargin(0, 0, 0, 10));

  // Quit Button
  QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("QuitButton"));
  UTextBlock* QuitText = WidgetTree->ConstructWidget<UTextBlock>();
  QuitText->SetText(FText::FromString(TEXT("QUITTER")));
  QuitText->SetFont(BtnFont);
  QuitText->SetJustification(ETextJustify::Center);
  QuitButton->AddChild(QuitText);
  UVerticalBoxSlot* QuitSlot = VBox->AddChildToVerticalBox(QuitButton);
  QuitSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
}

void UGameOverWidget::ShowResults(int32 FinalScore, int32 TotalKills,
                                   int32 BestCombo, bool bVictory) {
  // Titre et couleur selon victoire ou défaite
  if (TitleText) {
    if (bVictory) {
      TitleText->SetText(FText::FromString(TEXT("MISSION ACCOMPLIE")));
      TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.f, 0.2f)));
    } else {
      TitleText->SetText(FText::FromString(TEXT("VOUS ÊTES MORT")));
      TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.2f, 0.2f)));
    }
  }

  if (FinalScoreText) {
    FinalScoreText->SetText(
        FText::FromString(FString::Printf(TEXT("Score: %d"), FinalScore)));
  }

  if (TotalKillsText) {
    TotalKillsText->SetText(
        FText::FromString(FString::Printf(TEXT("Kills: %d"), TotalKills)));
  }

  if (BestComboText) {
    BestComboText->SetText(
        FText::FromString(FString::Printf(TEXT("Best Combo: x%d"), BestCombo)));
  }

  BP_OnResultsShown(FinalScore, TotalKills, BestCombo);
}

void UGameOverWidget::OnReplayClicked() {
  UGameplayStatics::OpenLevel(
      GetWorld(), FName(*GetWorld()->GetName()), true);
}

void UGameOverWidget::OnLeaderboardClicked() {
  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->ShowLeaderboard();
  }
}

void UGameOverWidget::OnQuitClicked() {
  UKismetSystemLibrary::QuitGame(
      GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
