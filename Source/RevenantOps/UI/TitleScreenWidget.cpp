// Copyright RevenantOps. All Rights Reserved.
#include "TitleScreenWidget.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
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

TSharedRef<SWidget> UTitleScreenWidget::RebuildWidget()
{
    if (WidgetTree && !WidgetTree->RootWidget)
    {
        BuildDefaultUI();
    }
    return Super::RebuildWidget();
}

void UTitleScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PlayButton)    PlayButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnPlayClicked);
    if (QuitButton)    QuitButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnQuitClicked);
    if (OptionsButton) OptionsButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnOptionsClicked);

    BindButtonSounds(PlayButton);
    BindButtonSounds(OptionsButton);
    BindButtonSounds(QuitButton);
}

// ─────────────────────────────────────────────────────────────────────────────
// BUILD DEFAULT UI — toutes les couleurs viennent de UUITheme
// ─────────────────────────────────────────────────────────────────────────────

void UTitleScreenWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;

    // ── Theme ─────────────────────────────────────────────────────────────────
    // Fallback si DA_Theme_Default n'existe pas encore en editor
    UUITheme* T = GetTheme();

    const FLinearColor Col_BgDeep    = T ? T->BgDeep      : FLinearColor(0.03f, 0.025f, 0.02f, 1.f);
    const FLinearColor Col_BgPanel   = T ? T->BgPanel      : FLinearColor(0.07f, 0.06f,  0.04f, 1.f);
    const FLinearColor Col_Red       = T ? T->RedBlood     : FLinearColor(0.75f, 0.15f,  0.10f, 1.f);
    const FLinearColor Col_Gold      = T ? T->GoldTarnish  : FLinearColor(0.85f, 0.70f,  0.30f, 1.f);
    const FLinearColor Col_GoldDim   = T ? T->GoldDim      : FLinearColor(0.55f, 0.45f,  0.20f, 1.f);
    const FLinearColor Col_White     = T ? T->WhiteText    : FLinearColor(0.95f, 0.93f,  0.88f, 1.f);
    const FLinearColor Col_Grey      = T ? T->GreySoft     : FLinearColor(0.45f, 0.42f,  0.38f, 1.f);
    const FLinearColor Col_Border    = T ? T->BorderColor  : FLinearColor(0.55f, 0.45f,  0.20f, 0.8f);
    const FLinearColor Col_BtnHov    = T ? T->ButtonBgHovered : FLinearColor(0.75f, 0.15f, 0.10f, 0.25f);
    const FLinearColor Col_BtnPress  = T ? T->ButtonBgPressed : FLinearColor(0.75f, 0.15f, 0.10f, 0.5f);

    // ── Helpers locaux ────────────────────────────────────────────────────────
    auto MakeBrush = [](const FLinearColor& C) -> FSlateBrush {
        FSlateBrush B; B.TintColor = FSlateColor(C); return B;
    };

    auto MakeBtnStyle = [&](const FLinearColor& BgN, const FLinearColor& BgH,
                             const FLinearColor& BgP) -> FButtonStyle
    {
        FButtonStyle S;
        S.SetNormal (MakeBrush(BgN));
        S.SetHovered(MakeBrush(BgH));
        S.SetPressed(MakeBrush(BgP));
        S.SetDisabled(MakeBrush(UUIHelpers::WithAlpha(Col_Grey, 0.4f)));
        S.NormalPadding  = FMargin(12.f, 10.f);
        S.PressedPadding = FMargin(12.f, 11.f, 12.f, 9.f);
        return S;
    };

    auto MakeText = [&](const FString& Str, int32 Size, const FLinearColor& Color,
                        ETextJustify::Type J = ETextJustify::Left) -> UTextBlock*
    {
        UTextBlock* Tb = WidgetTree->ConstructWidget<UTextBlock>();
        Tb->SetText(FText::FromString(Str));
        FSlateFontInfo Fi = Tb->GetFont();
        Fi.Size = Size;
        // Si le theme a une font Stencil configuree, l'utiliser pour les grands textes
        if (T && T->FontTitle.HasValidFont() && Size >= 24)  Fi = T->FontTitle;
        else if (T && T->FontBody.HasValidFont() && Size < 24) Fi = T->FontBody;
        Fi.Size = Size; // toujours forcer la taille demandee
        Tb->SetFont(Fi);
        Tb->SetColorAndOpacity(FSlateColor(Color));
        Tb->SetJustification(J);
        return Tb;
    };

    auto MakeSpacer = [&]() -> UTextBlock* {
        return WidgetTree->ConstructWidget<UTextBlock>();
    };

    // ── Root Canvas ───────────────────────────────────────────────────────────
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;

    // Fond sombre plein écran
    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(Col_BgDeep);
    Bg->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Vignette (overlay sombre sur les bords)
    UBorder* Vignette = WidgetTree->ConstructWidget<UBorder>();
    {
        const float VigAlpha = T ? T->VignetteOpacity : 0.35f;
        Vignette->SetBrushColor(UUIHelpers::WithAlpha(FLinearColor::Black, VigAlpha));
    }
    Vignette->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* VigSlot = Canvas->AddChildToCanvas(Vignette);
    VigSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    VigSlot->SetOffsets(FMargin(0.f));

    // Bande rouge haut (5px)
    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(Col_Red);
    TopBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* TopSlot = Canvas->AddChildToCanvas(TopBar);
    TopSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 5.f));
    TopSlot->SetAutoSize(true);

    // Séparateur vertical gauche/droite (60|40)
    UBorder* VSep = WidgetTree->ConstructWidget<UBorder>();
    VSep->SetBrushColor(UUIHelpers::WithAlpha(Col_Red, 0.5f));
    VSep->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* VSepSlot = Canvas->AddChildToCanvas(VSep);
    VSepSlot->SetAnchors(FAnchors(0.6f, 0.f, 0.6f, 1.f));
    VSepSlot->SetOffsets(FMargin(-1.f, 5.f, 1.f, 0.f));

    // ── Zone gauche — Titre ───────────────────────────────────────────────────
    UVerticalBox* LeftVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* LeftSlot = Canvas->AddChildToCanvas(LeftVBox);
    LeftSlot->SetAnchors(FAnchors(0.f, 0.f, 0.6f, 1.f));
    LeftSlot->SetOffsets(FMargin(80.f, 0.f, 0.f, 0.f));

    // Spacer haut — pousse le titre vers le centre
    UVerticalBoxSlot* SpacerTopSlot = LeftVBox->AddChildToVerticalBox(MakeSpacer());
    SpacerTopSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // "REVENANTOPS"
    UTextBlock* TitleTxt = MakeText(TEXT("REVENANTOPS"), 64, Col_Gold);
    UVerticalBoxSlot* TitleSlot = LeftVBox->AddChildToVerticalBox(TitleTxt);
    TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

    // Trait or sous le titre (2px)
    UBorder* TitleLine = WidgetTree->ConstructWidget<UBorder>();
    TitleLine->SetBrushColor(Col_Border);
    UVerticalBoxSlot* LineSlot = LeftVBox->AddChildToVerticalBox(TitleLine);
    LineSlot->SetPadding(FMargin(0.f, 0.f, 80.f, 14.f));
    if (UBorderSlot* BS = Cast<UBorderSlot>(TitleLine->AddChild(MakeSpacer())))
        BS->SetPadding(FMargin(0.f, 1.f));

    // "MODE MERCENAIRES"
    UTextBlock* SubTxt = MakeText(TEXT("MODE MERCENAIRES"), 16, Col_GoldDim);
    UVerticalBoxSlot* SubSlot = LeftVBox->AddChildToVerticalBox(SubTxt);
    SubSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 0.f));

    // Spacer bas
    UVerticalBoxSlot* SpacerBotSlot = LeftVBox->AddChildToVerticalBox(MakeSpacer());
    SpacerBotSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // Version build (bas gauche)
    UTextBlock* VersionTxt = MakeText(TEXT("v4.0 — BUILD 2026"), 11, Col_Grey);
    UVerticalBoxSlot* VerSlot = LeftVBox->AddChildToVerticalBox(VersionTxt);
    VerSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));

    // Copyright
    UTextBlock* Copyright = MakeText(TEXT("RevenantOps  © 2026"), 10, UUIHelpers::WithAlpha(Col_Grey, 0.6f));
    UVerticalBoxSlot* CopySlot = LeftVBox->AddChildToVerticalBox(Copyright);
    CopySlot->SetPadding(FMargin(0.f, 0.f, 0.f, 24.f));

    // ── Zone droite — Boutons ─────────────────────────────────────────────────
    UVerticalBox* RightVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* RightSlot = Canvas->AddChildToCanvas(RightVBox);
    RightSlot->SetAnchors(FAnchors(0.6f, 0.5f, 1.f, 0.5f));
    RightSlot->SetAlignment(FVector2D(0.f, 0.5f));
    RightSlot->SetOffsets(FMargin(40.f, -130.f, 60.f, 130.f));

    // Etiquette "OPERATIONS" au-dessus des boutons
    UTextBlock* MenuLabel = MakeText(TEXT("OPERATIONS"), 13, UUIHelpers::WithAlpha(Col_Border, 0.9f));
    UVerticalBoxSlot* LabelSlot = RightVBox->AddChildToVerticalBox(MenuLabel);
    LabelSlot->SetPadding(FMargin(4.f, 0.f, 0.f, 10.f));

    // Trait fin or sous le label
    UBorder* MenuLine = WidgetTree->ConstructWidget<UBorder>();
    MenuLine->SetBrushColor(UUIHelpers::WithAlpha(Col_Border, 0.4f));
    UVerticalBoxSlot* MenuLineSlot = RightVBox->AddChildToVerticalBox(MenuLine);
    MenuLineSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
    if (UBorderSlot* BS = Cast<UBorderSlot>(MenuLine->AddChild(MakeSpacer())))
        BS->SetPadding(FMargin(0.f, 1.f));

    // Helper ajout bouton
    auto AddBtn = [&](UButton*& BtnRef, const FString& Label,
                      const FLinearColor& BgN, const FLinearColor& BgH,
                      const FLinearColor& BgP, const FLinearColor& TextCol)
    {
        BtnRef = WidgetTree->ConstructWidget<UButton>();
        BtnRef->SetStyle(MakeBtnStyle(BgN, BgH, BgP));
        UTextBlock* Lbl = MakeText(Label, 18, TextCol, ETextJustify::Left);
        BtnRef->AddChild(Lbl);
        UVerticalBoxSlot* S = RightVBox->AddChildToVerticalBox(BtnRef);
        S->SetHorizontalAlignment(HAlign_Fill);
        S->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
    };

    // JOUER — rouge sang, texte blanc
    AddBtn(PlayButton,
        TEXT(">> DEPLOYER"),
        UUIHelpers::WithAlpha(Col_Red, 0.85f),
        Col_Red,
        UUIHelpers::WithAlpha(Col_Red, 0.6f),
        Col_White);

    // OPTIONS — panneau fonce, texte or
    AddBtn(OptionsButton,
        TEXT("   BRIEFING"),
        UUIHelpers::WithAlpha(Col_BgPanel, 0.9f),
        Col_BtnHov,
        Col_BtnPress,
        Col_Gold);

    // QUITTER — tres discret, texte gris
    AddBtn(QuitButton,
        TEXT("   ABANDONNER"),
        FLinearColor(0.f, 0.f, 0.f, 0.f),
        UUIHelpers::WithAlpha(Col_Grey, 0.15f),
        UUIHelpers::WithAlpha(Col_Grey, 0.3f),
        Col_Grey);
}

// ─────────────────────────────────────────────────────────────────────────────
// ACTIONS
// ─────────────────────────────────────────────────────────────────────────────

void UTitleScreenWidget::OnPlayClicked()
{
    PlayClickSound();
    BP_OnPlayPressed();

    if (ARevenantOpsPlayerController* PC =
            Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
    {
        PC->ShowLevelSelectScreen();
    }
}

void UTitleScreenWidget::OnOptionsClicked()
{
    PlayClickSound();

    if (ARevenantOpsPlayerController* PC =
            Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
    {
        PC->ShowOptionsScreen();
    }
}

void UTitleScreenWidget::OnQuitClicked()
{
    PlayClickSound();
    UKismetSystemLibrary::QuitGame(
        GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
