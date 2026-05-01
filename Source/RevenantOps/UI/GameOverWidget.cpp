// Copyright RevenantOps. All Rights Reserved.

#include "GameOverWidget.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
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
    ReplayButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnReplayClicked);
    BindButtonSounds(ReplayButton);
  }
  if (LeaderboardButton) {
    LeaderboardButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnLeaderboardClicked);
    BindButtonSounds(LeaderboardButton);
  }
  if (QuitButton) {
    QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
    BindButtonSounds(QuitButton);
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

  // ── Theme ────────────────────────────────────────────────────────────────
  UUITheme* T = GetTheme();
  const FLinearColor C_Gold    = T ? T->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
  const FLinearColor C_GoldDim = T ? T->GoldDim     : FLinearColor(0.55f, 0.45f, 0.20f, 1.f);
  const FLinearColor C_White   = T ? T->WhiteText   : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
  const FLinearColor C_Grey    = T ? T->GreySoft    : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
  const FLinearColor C_Red     = T ? T->RedBlood    : FLinearColor(0.75f, 0.15f, 0.10f, 1.f);
  const FLinearColor C_Alert   = T ? T->RedAlert    : FLinearColor(1.f,   0.25f, 0.15f, 1.f);
  const FLinearColor C_Panel   = T ? T->BgPanel     : FLinearColor(0.07f, 0.06f, 0.04f, 1.f);

  auto MakeBtnStyle = [&](const FLinearColor& N, const FLinearColor& H) {
      FButtonStyle S;
      S.SetNormal (UUIHelpers::MakeSolidBrush(N));
      S.SetHovered(UUIHelpers::MakeSolidBrush(H));
      S.SetPressed(UUIHelpers::MakeSolidBrush(UUIHelpers::WithAlpha(H, 0.6f)));
      S.NormalPadding  = FMargin(32.f, 10.f);
      S.PressedPadding = FMargin(32.f, 11.f, 32.f, 9.f);
      return S;
  };

  FSlateFontInfo LargeFont = UUIHelpers::GetFont(T, 32);
  FSlateFontInfo MedFont   = UUIHelpers::GetFont(T, 20);
  FSlateFontInfo BtnFont   = UUIHelpers::GetFont(T, 16);
  FSlateFontInfo SmallFont = UUIHelpers::GetFont(T, 14);

  UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
  UCanvasPanelSlot* VBoxSlot = Canvas->AddChildToCanvas(VBox);
  VBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
  VBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
  VBoxSlot->SetAutoSize(true);

  auto AddStat = [&](UTextBlock*& OutTb, const FString& InitStr,
                     FSlateFontInfo Font, const FLinearColor& Color, FMargin Pad)
  {
      OutTb = WidgetTree->ConstructWidget<UTextBlock>();
      OutTb->SetText(FText::FromString(InitStr));
      OutTb->SetFont(Font);
      OutTb->SetColorAndOpacity(FSlateColor(Color));
      OutTb->SetJustification(ETextJustify::Center);
      UVerticalBoxSlot* S = VBox->AddChildToVerticalBox(OutTb);
      S->SetHorizontalAlignment(HAlign_Center);
      S->SetPadding(Pad);
  };

  // "RAPPORT DE MISSION" header
  {
      UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>();
      Header->SetText(FText::FromString(TEXT("RAPPORT DE MISSION")));
      Header->SetFont(UUIHelpers::GetFont(T, 13));
      Header->SetColorAndOpacity(FSlateColor(C_GoldDim));
      Header->SetJustification(ETextJustify::Center);
      VBox->AddChildToVerticalBox(Header)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
  }

  // Trait séparateur
  {
      UImage* Line = WidgetTree->ConstructWidget<UImage>();
      Line->SetColorAndOpacity(UUIHelpers::WithAlpha(C_Red, 0.6f));
      VBox->AddChildToVerticalBox(Line)->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
  }

  // Titre (mort / victoire) — mis à jour dans ShowResults
  TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
  TitleText->SetText(FText::FromString(TEXT("OPERATEUR NEUTRALISE")));
  TitleText->SetFont(LargeFont);
  TitleText->SetColorAndOpacity(FSlateColor(C_Alert));
  TitleText->SetJustification(ETextJustify::Center);
  VBox->AddChildToVerticalBox(TitleText)->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));

  // ── Tier rang S/A/B/C/D ─────────────────────────────────────────────────
  {
      // Sous-label "EVALUATION"
      RankLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RankLabelText"));
      RankLabelText->SetText(FText::FromString(TEXT("EVALUATION")));
      RankLabelText->SetFont(UUIHelpers::GetFont(T, 11));
      RankLabelText->SetColorAndOpacity(FSlateColor(C_GoldDim));
      RankLabelText->SetJustification(ETextJustify::Center);
      VBox->AddChildToVerticalBox(RankLabelText)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

      // Grande lettre du rang
      RankText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RankText"));
      RankText->SetText(FText::FromString(TEXT("D")));
      RankText->SetFont(UUIHelpers::GetFont(T, 96));  // très grande
      RankText->SetColorAndOpacity(FSlateColor(C_Grey));
      RankText->SetJustification(ETextJustify::Center);
      VBox->AddChildToVerticalBox(RankText)->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
  }

  // Stats
  AddStat(FinalScoreText, TEXT("SCORE : 0"),          MedFont, C_Gold,    FMargin(0.f, 0.f, 0.f, 8.f));
  AddStat(TotalKillsText, TEXT("ELIMINATIONS : 0"),   MedFont, C_White,   FMargin(0.f, 0.f, 0.f, 8.f));
  AddStat(BestComboText,  TEXT("MEILLEUR COMBO : x0"), MedFont, C_GoldDim, FMargin(0.f, 0.f, 0.f, 28.f));

  // Boutons
  auto AddBtn = [&](UButton*& BtnRef, const FString& Label,
                    const FLinearColor& BgN, const FLinearColor& BgH, FMargin Pad)
  {
      BtnRef = WidgetTree->ConstructWidget<UButton>();
      BtnRef->SetStyle(MakeBtnStyle(BgN, BgH));
      UTextBlock* Lbl = WidgetTree->ConstructWidget<UTextBlock>();
      Lbl->SetText(FText::FromString(Label));
      Lbl->SetFont(BtnFont);
      Lbl->SetJustification(ETextJustify::Center);
      Lbl->SetColorAndOpacity(FSlateColor(C_White));
      BtnRef->AddChild(Lbl);
      VBox->AddChildToVerticalBox(BtnRef)->SetPadding(Pad);
  };

  AddBtn(ReplayButton,      TEXT(">> REDEPLOY"),          UUIHelpers::WithAlpha(C_Red, 0.85f), C_Red,   FMargin(0.f, 0.f, 0.f, 8.f));
  AddBtn(LeaderboardButton, TEXT("   CLASSEMENT"),        UUIHelpers::WithAlpha(C_Panel, 0.9f), UUIHelpers::WithAlpha(C_Gold, 0.2f), FMargin(0.f, 0.f, 0.f, 8.f));
  AddBtn(QuitButton,        TEXT("   RETOUR AU QG"),      FLinearColor(0.f, 0.f, 0.f, 0.f),   UUIHelpers::WithAlpha(C_Grey, 0.15f), FMargin(0.f, 0.f, 0.f, 0.f));
}

FString UGameOverWidget::ComputeRank(int32 Score, int32 BestCombo)
{
    // Bonus combo : +500 par niveau de combo au-dessus de x1
    const int32 ComboBonus = FMath::Max(0, BestCombo - 1) * 500;
    const int32 Adjusted   = Score + ComboBonus;

    if (Adjusted >= 5000) return TEXT("S");
    if (Adjusted >= 3000) return TEXT("A");
    if (Adjusted >= 1500) return TEXT("B");
    if (Adjusted >= 500)  return TEXT("C");
    return TEXT("D");
}

void UGameOverWidget::ShowResults(int32 FinalScore, int32 TotalKills,
                                   int32 BestCombo, bool bVictory)
{
    UUITheme* T = GetTheme();
    const FLinearColor C_Gold   = T ? T->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
    const FLinearColor C_White  = T ? T->WhiteText   : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
    const FLinearColor C_GoldDim= T ? T->GoldDim     : FLinearColor(0.55f, 0.45f, 0.20f, 1.f);
    const FLinearColor C_Grey   = T ? T->GreySoft    : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
    const FLinearColor C_Alert  = T ? T->RedAlert    : FLinearColor(1.f,   0.25f, 0.15f, 1.f);
    const FLinearColor C_Red    = T ? T->RedBlood    : FLinearColor(0.75f, 0.15f, 0.10f, 1.f);

    if (TitleText)
    {
        if (bVictory)
        {
            TitleText->SetText(FText::FromString(TEXT("MISSION ACCOMPLIE")));
            TitleText->SetColorAndOpacity(FSlateColor(C_Gold));
        }
        else
        {
            TitleText->SetText(FText::FromString(TEXT("OPERATEUR NEUTRALISE")));
            TitleText->SetColorAndOpacity(FSlateColor(C_Alert));
        }
    }

    // ── Rang S/A/B/C/D ───────────────────────────────────────────────────
    if (RankText)
    {
        const FString Rank = ComputeRank(FinalScore, BestCombo);
        RankText->SetText(FText::FromString(Rank));

        // Couleur par rang
        FLinearColor RankColor;
        if      (Rank == TEXT("S")) RankColor = C_Gold;    // or terni brillant
        else if (Rank == TEXT("A")) RankColor = C_White;   // blanc cassé
        else if (Rank == TEXT("B")) RankColor = C_GoldDim; // or éteint
        else if (Rank == TEXT("C")) RankColor = C_Grey;    // gris
        else                        RankColor = C_Red;     // rouge sang (D)

        RankText->SetColorAndOpacity(FSlateColor(RankColor));
    }

    if (FinalScoreText)
        FinalScoreText->SetText(FText::FromString(
            FString::Printf(TEXT("SCORE : %d"), FinalScore)));

    if (TotalKillsText)
        TotalKillsText->SetText(FText::FromString(
            FString::Printf(TEXT("ELIMINATIONS : %d"), TotalKills)));

    if (BestComboText)
        BestComboText->SetText(FText::FromString(
            FString::Printf(TEXT("MEILLEUR COMBO : x%d"), BestCombo)));

    BP_OnResultsShown(FinalScore, TotalKills, BestCombo);
}

void UGameOverWidget::OnReplayClicked() {
  UGameplayStatics::SetGamePaused(GetWorld(), false);
  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->RestartMatch();
  }
}

void UGameOverWidget::OnLeaderboardClicked() {
  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->ShowLeaderboard();
  }
}

void UGameOverWidget::OnQuitClicked() {
  UGameplayStatics::SetGamePaused(GetWorld(), false);
  UGameplayStatics::OpenLevel(GetWorld(), FName("Lvl_MainMenu"), true);
}
