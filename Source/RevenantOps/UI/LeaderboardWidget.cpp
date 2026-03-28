// Copyright RevenantOps. All Rights Reserved.

#include "LeaderboardWidget.h"
#include "Styling/CoreStyle.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "RevenantOpsPlayerController.h"
#include "LeaderboardSaveGame.h"

TSharedRef<SWidget> ULeaderboardWidget::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    BuildDefaultUI();
  }
  return Super::RebuildWidget();
}

void ULeaderboardWidget::NativeConstruct() {
  Super::NativeConstruct();

  if (BackButton) {
    BackButton->OnClicked.AddDynamic(this,
                                      &ULeaderboardWidget::OnBackClicked);
  }

  LoadScores();
}

void ULeaderboardWidget::BuildDefaultUI() {
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

  FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 36);
  FSlateFontInfo BtnFont   = FCoreStyle::GetDefaultFontStyle("Regular", 20);

  // Title
  UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
  Title->SetText(FText::FromString(TEXT("LEADERBOARD")));
  Title->SetFont(TitleFont);
  Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.8f, 0.2f)));
  Title->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(Title);
  TitleSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  TitleSlot->SetPadding(FMargin(0, 0, 0, 20));

  // Score list
  ScoreListBox = WidgetTree->ConstructWidget<UVerticalBox>();
  UVerticalBoxSlot* ListSlot = VBox->AddChildToVerticalBox(ScoreListBox);
  ListSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  ListSlot->SetPadding(FMargin(0, 0, 0, 20));

  // Back Button
  BackButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BackButton"));
  UTextBlock* BackText = WidgetTree->ConstructWidget<UTextBlock>();
  BackText->SetText(FText::FromString(TEXT("RETOUR")));
  BackText->SetFont(BtnFont);
  BackText->SetJustification(ETextJustify::Center);
  BackButton->AddChild(BackText);
  UVerticalBoxSlot* BackSlot = VBox->AddChildToVerticalBox(BackButton);
  BackSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
}

void ULeaderboardWidget::RebuildScoreList() {
  if (!ScoreListBox) return;

  ScoreListBox->ClearChildren();

  FSlateFontInfo EntryFont = FCoreStyle::GetDefaultFontStyle("Regular", 16);

  if (Scores.Num() == 0) {
    UTextBlock* Empty = NewObject<UTextBlock>(this);
    Empty->SetText(FText::FromString(TEXT("Pas encore de scores")));
    Empty->SetFont(EntryFont);
    Empty->SetJustification(ETextJustify::Center);
    ScoreListBox->AddChild(Empty);
    return;
  }

  for (int32 i = 0; i < Scores.Num() && i < MaxEntries; ++i) {
    const FScoreEntry& E = Scores[i];
    FString Line = FString::Printf(TEXT("#%d  %d pts — %d kills — x%d combo"),
                                    i + 1, E.Score, E.Kills, E.BestCombo);
    UTextBlock* Entry = NewObject<UTextBlock>(this);
    Entry->SetText(FText::FromString(Line));
    Entry->SetFont(EntryFont);
    Entry->SetJustification(ETextJustify::Center);
    UVerticalBoxSlot* EntrySlot = ScoreListBox->AddChildToVerticalBox(Entry);
    EntrySlot->SetPadding(FMargin(0, 2, 0, 2));
  }
}

void ULeaderboardWidget::OnBackClicked() {
  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->ShowTitleScreen();
  }
}

void ULeaderboardWidget::AddScore(int32 Score, int32 Kills, int32 BestCombo) {
  FScoreEntry Entry;
  Entry.Score = Score;
  Entry.Kills = Kills;
  Entry.BestCombo = BestCombo;
  Entry.Date = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M"));

  Scores.Add(Entry);

  Scores.Sort([](const FScoreEntry &A, const FScoreEntry &B) {
    return A.Score > B.Score;
  });

  if (Scores.Num() > MaxEntries) {
    Scores.SetNum(MaxEntries);
  }

  SaveScores();
  RebuildScoreList();
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

  RebuildScoreList();
  BP_OnScoresUpdated();
}

void ULeaderboardWidget::SaveScoreStatic(UObject* WorldContext, int32 Score, int32 Kills, int32 BestCombo,
                                          const FString& SlotName, int32 MaxEntries)
{
  ULeaderboardSaveGame* SaveGame = Cast<ULeaderboardSaveGame>(
      UGameplayStatics::LoadGameFromSlot(SlotName, 0));

  if (!SaveGame)
  {
    SaveGame = Cast<ULeaderboardSaveGame>(
        UGameplayStatics::CreateSaveGameObject(ULeaderboardSaveGame::StaticClass()));
  }

  if (!SaveGame) { return; }

  FScoreEntry Entry;
  Entry.Score     = Score;
  Entry.Kills     = Kills;
  Entry.BestCombo = BestCombo;
  Entry.Date      = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M"));
  SaveGame->Scores.Add(Entry);

  SaveGame->Scores.Sort([](const FScoreEntry& A, const FScoreEntry& B) {
    return A.Score > B.Score;
  });

  if (SaveGame->Scores.Num() > MaxEntries)
  {
    SaveGame->Scores.SetNum(MaxEntries);
  }

  UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
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
