// Copyright RevenantOps. All Rights Reserved.

#include "LeaderboardWidget.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
#include "UI/GameOverWidget.h"
#include "Styling/CoreStyle.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
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
    BackButton->OnClicked.AddDynamic(this, &ULeaderboardWidget::OnBackClicked);
    BindButtonSounds(BackButton);
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

  // ── Theme ─────────────────────────────────────────────────────────────
  UUITheme* T = GetTheme();
  const FLinearColor C_Bg      = T ? T->BgPanelDim  : FLinearColor(0.05f, 0.04f, 0.03f, 0.92f);
  const FLinearColor C_Gold    = T ? T->GoldTarnish  : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
  const FLinearColor C_GoldDim = T ? T->GoldDim      : FLinearColor(0.55f, 0.45f, 0.20f, 1.f);
  const FLinearColor C_White   = T ? T->WhiteText    : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
  const FLinearColor C_Grey    = T ? T->GreySoft     : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
  const FLinearColor C_Red     = T ? T->RedBlood     : FLinearColor(0.75f, 0.15f, 0.10f, 1.f);
  const FLinearColor C_Panel   = T ? T->BgPanel      : FLinearColor(0.07f, 0.06f, 0.04f, 1.f);
  const FLinearColor C_Deep    = T ? T->BgDeep       : FLinearColor(0.03f, 0.025f, 0.02f, 1.f);

  // Full-screen fond profond
  UImage* Background = WidgetTree->ConstructWidget<UImage>();
  Background->SetColorAndOpacity(UUIHelpers::WithAlpha(C_Deep, 0.96f));
  UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(Background);
  BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
  BgSlot->SetOffsets(FMargin(0.f));
  BgSlot->SetZOrder(0);

  // Panneau centré avec fond panel
  UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
  Panel->SetBrushColor(C_Bg);
  Panel->SetPadding(FMargin(48.f, 36.f));
  UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
  PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
  PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
  PanelSlot->SetAutoSize(true);
  PanelSlot->SetZOrder(1);

  UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
  Panel->SetContent(VBox);

  // ── Sous-titre "CLASSEMENT GLOBAL" ───────────────────────────────────
  {
    UTextBlock* Sub = WidgetTree->ConstructWidget<UTextBlock>();
    Sub->SetText(FText::FromString(TEXT("CLASSEMENT GLOBAL")));
    Sub->SetFont(UUIHelpers::GetFont(T, 11));
    Sub->SetColorAndOpacity(FSlateColor(C_GoldDim));
    Sub->SetJustification(ETextJustify::Center);
    VBox->AddChildToVerticalBox(Sub)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
  }

  // ── Titre "LEADERBOARD" ───────────────────────────────────────────────
  {
    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
    Title->SetText(FText::FromString(TEXT("LEADERBOARD")));
    Title->SetFont(UUIHelpers::GetFont(T, 36));
    Title->SetColorAndOpacity(FSlateColor(C_Gold));
    Title->SetJustification(ETextJustify::Center);
    VBox->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
  }

  // Séparateur rouge sang
  {
    UImage* Line = WidgetTree->ConstructWidget<UImage>();
    Line->SetColorAndOpacity(UUIHelpers::WithAlpha(C_Red, 0.7f));
    VBox->AddChildToVerticalBox(Line)->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
  }

  // ── Liste des scores ──────────────────────────────────────────────────
  ScoreListBox = WidgetTree->ConstructWidget<UVerticalBox>();
  VBox->AddChildToVerticalBox(ScoreListBox)->SetPadding(FMargin(0.f, 0.f, 0.f, 24.f));

  // ── Bouton RETOUR ─────────────────────────────────────────────────────
  BackButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BackButton"));
  {
    FButtonStyle BtnStyle = UUIHelpers::MakeStandardButtonStyle(T);
    BackButton->SetStyle(BtnStyle);
  }
  UTextBlock* BackText = WidgetTree->ConstructWidget<UTextBlock>();
  BackText->SetText(FText::FromString(TEXT("< RETOUR")));
  BackText->SetFont(UUIHelpers::GetFont(T, 15));
  BackText->SetColorAndOpacity(FSlateColor(C_GoldDim));
  BackText->SetJustification(ETextJustify::Center);
  BackButton->AddChild(BackText);
  VBox->AddChildToVerticalBox(BackButton)->SetHorizontalAlignment(HAlign_Center);
}

void ULeaderboardWidget::RebuildScoreList() {
  if (!ScoreListBox || !WidgetTree) return;

  ScoreListBox->ClearChildren();

  UUITheme* T = GetTheme();
  const FLinearColor C_Gold    = T ? T->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
  const FLinearColor C_GoldDim = T ? T->GoldDim     : FLinearColor(0.55f, 0.45f, 0.20f, 1.f);
  const FLinearColor C_White   = T ? T->WhiteText   : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
  const FLinearColor C_Grey    = T ? T->GreySoft    : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
  const FLinearColor C_Red     = T ? T->RedBlood    : FLinearColor(0.75f, 0.15f, 0.10f, 1.f);
  const FLinearColor C_Alert   = T ? T->RedAlert    : FLinearColor(1.f, 0.25f, 0.15f, 1.f);

  if (Scores.Num() == 0) {
    UTextBlock* Empty = WidgetTree->ConstructWidget<UTextBlock>();
    Empty->SetText(FText::FromString(TEXT("Aucun score enregistré")));
    Empty->SetFont(UUIHelpers::GetFont(T, 16));
    Empty->SetColorAndOpacity(FSlateColor(C_Grey));
    Empty->SetJustification(ETextJustify::Center);
    ScoreListBox->AddChild(Empty);
    return;
  }

  for (int32 i = 0; i < Scores.Num() && i < MaxEntries; ++i) {
    const FScoreEntry& E = Scores[i];
    const FString Rank   = UGameOverWidget::ComputeRank(E.Score, E.BestCombo);

    // Couleur rang
    FLinearColor RankColor;
    if      (Rank == TEXT("S")) RankColor = C_Gold;
    else if (Rank == TEXT("A")) RankColor = C_White;
    else if (Rank == TEXT("B")) RankColor = C_GoldDim;
    else if (Rank == TEXT("C")) RankColor = C_Grey;
    else                        RankColor = C_Red;

    // Ligne horizontale : [#N] [rang] [score] [kills] [combo]
    UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();

    auto AddCol = [&](const FString& Str, const FLinearColor& Col, int32 Size,
                      EHorizontalAlignment Align, float PadRight) {
      UTextBlock* Tb = WidgetTree->ConstructWidget<UTextBlock>();
      Tb->SetText(FText::FromString(Str));
      Tb->SetFont(UUIHelpers::GetMonoFont(T, Size));
      Tb->SetColorAndOpacity(FSlateColor(Col));
      Tb->SetJustification(ETextJustify::Right);
      UHorizontalBoxSlot* HS = HBox->AddChildToHorizontalBox(Tb);
      HS->SetVerticalAlignment(VAlign_Center);
      HS->SetPadding(FMargin(0.f, 0.f, PadRight, 0.f));
    };

    AddCol(FString::Printf(TEXT("#%d"), i + 1),         C_GoldDim, 14, HAlign_Right, 12.f);
    AddCol(Rank,                                          RankColor, 16, HAlign_Center, 12.f);
    AddCol(FString::Printf(TEXT("%6d pts"), E.Score),    C_Gold,    14, HAlign_Right,  8.f);
    AddCol(FString::Printf(TEXT("— %d elim"), E.Kills), C_White,   14, HAlign_Right,  8.f);
    AddCol(FString::Printf(TEXT("— x%d"), E.BestCombo), C_GoldDim, 14, HAlign_Right,  0.f);

    UVerticalBoxSlot* EntrySlot = ScoreListBox->AddChildToVerticalBox(HBox);
    EntrySlot->SetPadding(FMargin(0.f, 3.f, 0.f, 3.f));
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
