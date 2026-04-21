// Copyright RevenantOps. All Rights Reserved.

#include "TitleScreenWidget.h"
#include "Styling/CoreStyle.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RevenantOpsPlayerController.h"

TSharedRef<SWidget> UTitleScreenWidget::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    BuildDefaultUI();
  }
  return Super::RebuildWidget();
}

void UTitleScreenWidget::NativeConstruct() {
  Super::NativeConstruct();

  if (PlayButton) {
    PlayButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnPlayClicked);
  }

  if (QuitButton) {
    QuitButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnQuitClicked);
  }

  if (OptionsButton) {
    OptionsButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnOptionsClicked);
  }
}

void UTitleScreenWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;

    // ── Helpers locaux ────────────────────────────────────────────────────────
    const FLinearColor TS_Bg      (0.04f, 0.03f, 0.02f, 1.f);
    const FLinearColor TS_Panel   (0.07f, 0.06f, 0.04f, 1.f);
    const FLinearColor TS_Red     (0.75f, 0.15f, 0.10f, 1.f);
    const FLinearColor TS_Gold    (0.85f, 0.70f, 0.30f, 1.f);
    const FLinearColor TS_GoldDim (0.55f, 0.45f, 0.20f, 1.f);
    const FLinearColor TS_White   (1.f,   1.f,   1.f,   1.f);
    const FLinearColor TS_Grey    (0.45f, 0.42f, 0.38f, 1.f);

    auto MakeBrush = [](FLinearColor C) {
        FSlateBrush B; B.TintColor = FSlateColor(C); return B;
    };
    auto MakeBtnStyle = [&](FLinearColor N, FLinearColor H) {
        FButtonStyle S;
        FSlateBrush BN; BN.TintColor = FSlateColor(N);
        FSlateBrush BH; BH.TintColor = FSlateColor(H);
        FSlateBrush BP; BP.TintColor = FSlateColor(N * 0.7f);
        S.SetNormal(BN); S.SetHovered(BH); S.SetPressed(BP);
        return S;
    };
    auto MakeText = [&](const FString& Str, int32 Size, FLinearColor Color,
                        ETextJustify::Type J = ETextJustify::Left) -> UTextBlock*
    {
        UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
        T->SetText(FText::FromString(Str));
        FSlateFontInfo F = T->GetFont(); F.Size = Size; T->SetFont(F);
        T->SetColorAndOpacity(FSlateColor(Color));
        T->SetJustification(J);
        return T;
    };

    // ── Root ─────────────────────────────────────────────────────────────────
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;

    // Fond sombre plein écran
    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(TS_Bg);
    Bg->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Bande rouge haut
    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(TS_Red);
    TopBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* TopSlot = Canvas->AddChildToCanvas(TopBar);
    TopSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 5.f));
    TopSlot->SetAutoSize(true);

    // Bande verticale séparatrice gauche/droite (60% | 40%)
    UBorder* VSep = WidgetTree->ConstructWidget<UBorder>();
    VSep->SetBrushColor(FLinearColor(TS_Red.R, TS_Red.G, TS_Red.B, 0.6f));
    VSep->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* VSepSlot = Canvas->AddChildToCanvas(VSep);
    VSepSlot->SetAnchors(FAnchors(0.6f, 0.f, 0.6f, 1.f));
    VSepSlot->SetOffsets(FMargin(-1.f, 5.f, 1.f, 0.f));

    // ── Zone gauche — Titre ───────────────────────────────────────────────────
    UVerticalBox* LeftVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* LeftSlot = Canvas->AddChildToCanvas(LeftVBox);
    LeftSlot->SetAnchors(FAnchors(0.f, 0.f, 0.6f, 1.f));
    LeftSlot->SetOffsets(FMargin(80.f, 0.f, 0.f, 0.f));
    LeftSlot->SetAlignment(FVector2D(0.f, 0.f));

    // Spacer haut
    UVerticalBoxSlot* SpacerTop = LeftVBox->AddChildToVerticalBox(
        WidgetTree->ConstructWidget<UTextBlock>());
    SpacerTop->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // Titre du jeu
    UTextBlock* TitleTxt = MakeText(TEXT("REVENANTOPS"), 64, TS_Gold);
    FSlateFontInfo TF = TitleTxt->GetFont();
    TF.Size = 64; TitleTxt->SetFont(TF);
    UVerticalBoxSlot* TitleSlot = LeftVBox->AddChildToVerticalBox(TitleTxt);
    TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // Trait rouge sous le titre
    UBorder* TitleLine = WidgetTree->ConstructWidget<UBorder>();
    TitleLine->SetBrushColor(TS_Red);
    UVerticalBoxSlot* LineSlot = LeftVBox->AddChildToVerticalBox(TitleLine);
    LineSlot->SetPadding(FMargin(0.f, 0.f, 80.f, 16.f));
    Cast<UBorderSlot>(TitleLine->AddChild(
        WidgetTree->ConstructWidget<UTextBlock>()))->SetPadding(FMargin(0.f, 2.f));

    // Sous-titre
    UTextBlock* SubTxt = MakeText(TEXT("MODE MERCENAIRES"), 18, TS_GoldDim);
    UVerticalBoxSlot* SubSlot = LeftVBox->AddChildToVerticalBox(SubTxt);
    SubSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 0.f));

    // Spacer bas
    UVerticalBoxSlot* SpacerBot = LeftVBox->AddChildToVerticalBox(
        WidgetTree->ConstructWidget<UTextBlock>());
    SpacerBot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // Copyright bas gauche
    UTextBlock* Copyright = MakeText(TEXT("RevenantOps  2026"), 11, TS_Grey);
    UVerticalBoxSlot* CopySlot = LeftVBox->AddChildToVerticalBox(Copyright);
    CopySlot->SetPadding(FMargin(0.f, 0.f, 0.f, 24.f));

    // ── Zone droite — Boutons ─────────────────────────────────────────────────
    UVerticalBox* RightVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* RightSlot = Canvas->AddChildToCanvas(RightVBox);
    RightSlot->SetAnchors(FAnchors(0.6f, 0.5f, 1.f, 0.5f));
    RightSlot->SetAlignment(FVector2D(0.f, 0.5f));
    RightSlot->SetOffsets(FMargin(40.f, -120.f, 60.f, 120.f));

    // Fonction helper bouton
    auto AddBtn = [&](UButton*& BtnRef, const FString& Label,
                      FLinearColor BgColor, FLinearColor HoverColor, FLinearColor TextColor)
    {
        BtnRef = WidgetTree->ConstructWidget<UButton>();
        BtnRef->SetStyle(MakeBtnStyle(BgColor, HoverColor));
        UTextBlock* Lbl = MakeText(Label, 20, TextColor, ETextJustify::Center);
        BtnRef->AddChild(Lbl);
        UVerticalBoxSlot* S = RightVBox->AddChildToVerticalBox(BtnRef);
        S->SetHorizontalAlignment(HAlign_Fill);
        S->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
    };

    AddBtn(PlayButton,    TEXT("JOUER"),    TS_Red,   FLinearColor(0.9f, 0.2f, 0.15f, 1.f), TS_White);
    AddBtn(OptionsButton, TEXT("OPTIONS"),  TS_Panel, FLinearColor(0.14f, 0.12f, 0.09f, 1.f), TS_Gold);
    AddBtn(QuitButton,    TEXT("QUITTER"),  FLinearColor(0.05f, 0.04f, 0.03f, 1.f),
                                            FLinearColor(0.10f, 0.08f, 0.06f, 1.f), TS_Grey);
}

void UTitleScreenWidget::OnPlayClicked() {
  BP_OnPlayPressed();

  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->ShowLevelSelectScreen();
  }
}

void UTitleScreenWidget::OnOptionsClicked() {
  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->ShowOptionsScreen();
  }
}

void UTitleScreenWidget::OnQuitClicked() {
  UKismetSystemLibrary::QuitGame(
      GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
