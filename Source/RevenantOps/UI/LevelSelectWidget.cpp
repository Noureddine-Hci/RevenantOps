// Copyright RevenantOps. All Rights Reserved.
#include "UI/LevelSelectWidget.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LeaderboardSaveGame.h"

// ─── Helpers fichier-local ────────────────────────────────────────────────────
static FSlateBrush LSW_MakeBrush(const FLinearColor& Color)
{
    FSlateBrush B; B.TintColor = FSlateColor(Color); return B;
}

static FButtonStyle LSW_MakeButtonStyle(const FLinearColor& Normal, const FLinearColor& Hover)
{
    FButtonStyle S;
    S.SetNormal (LSW_MakeBrush(Normal));
    S.SetHovered(LSW_MakeBrush(Hover));
    S.SetPressed(LSW_MakeBrush(Normal * 0.7f));
    return S;
}

static UTextBlock* LSW_MakeText(UWidgetTree* WT, const FString& Str, int32 Size,
                                 const FLinearColor& Color,
                                 ETextJustify::Type Justify = ETextJustify::Left)
{
    UTextBlock* T = WT->ConstructWidget<UTextBlock>();
    T->SetText(FText::FromString(Str));
    FSlateFontInfo F = T->GetFont(); F.Size = Size; T->SetFont(F);
    T->SetColorAndOpacity(FSlateColor(Color));
    T->SetJustification(Justify);
    return T;
}

// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> ULevelSelectWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt)
        BuildDefaultUI();
    return Super::RebuildWidget();
}

void ULevelSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();
    UE_LOG(LogTemp, Warning, TEXT("[LevelSelect] NativeConstruct — CarouselBox=%s BtnPrev=%s BtnNext=%s BtnConfirm=%s BtnBack=%s"),
        CarouselBox ? TEXT("OK") : TEXT("null"),
        BtnPrev ? TEXT("OK") : TEXT("null"),
        BtnNext ? TEXT("OK") : TEXT("null"),
        BtnConfirm ? TEXT("OK") : TEXT("null"),
        BtnBack ? TEXT("OK") : TEXT("null"));
    if (BtnBack)    BtnBack->OnClicked.AddDynamic(this,    &ULevelSelectWidget::HandleBack);
    if (BtnPrev)    BtnPrev->OnClicked.AddDynamic(this,    &ULevelSelectWidget::HandlePrev);
    if (BtnNext)    BtnNext->OnClicked.AddDynamic(this,    &ULevelSelectWidget::HandleNext);
    if (BtnConfirm) BtnConfirm->OnClicked.AddDynamic(this, &ULevelSelectWidget::HandleConfirm);

    BindButtonSounds(BtnBack);
    BindButtonSounds(BtnPrev);
    BindButtonSounds(BtnNext);
    BindButtonSounds(BtnConfirm);

    if (!CachedLevels.IsEmpty())
        PopulateLevels(CachedLevels);

}

// ─────────────────────────────────────────────────────────────────────────────

void ULevelSelectWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    // ── Theme ────────────────────────────────────────────────────────────────
    UUITheme* T = GetTheme();
    const FLinearColor C_BgDark    = T ? T->BgDeep     : FLinearColor(0.03f, 0.025f, 0.02f, 1.f);
    const FLinearColor C_PanelDark = T ? T->BgPanel     : FLinearColor(0.07f, 0.06f,  0.04f, 1.f);
    const FLinearColor C_PanelMid  = FLinearColor(0.10f, 0.09f, 0.07f, 1.f); // intermédiaire
    const FLinearColor C_Gold      = T ? T->GoldTarnish : FLinearColor(0.85f, 0.70f,  0.30f, 1.f);
    const FLinearColor C_GoldDim   = T ? T->GoldDim     : FLinearColor(0.55f, 0.45f,  0.20f, 1.f);
    const FLinearColor C_White     = T ? T->WhiteText   : FLinearColor(0.95f, 0.93f,  0.88f, 1.f);
    const FLinearColor C_Grey      = T ? T->GreySoft    : FLinearColor(0.45f, 0.42f,  0.38f, 1.f);
    const FLinearColor C_Red       = T ? T->RedBlood    : FLinearColor(0.75f, 0.15f,  0.10f, 1.f);

    // ── Root canvas ──────────────────────────────────────────────────────────
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(C_BgDark);
    Bg->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(C_Red);
    TopBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* TopBarSlot = Root->AddChildToCanvas(TopBar);
    TopBarSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopBarSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 4.f));
    TopBarSlot->SetAutoSize(true);

    // ── Layout vertical principal ─────────────────────────────────────────────
    UVerticalBox* VMain = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VMainSlot = Root->AddChildToCanvas(VMain);
    VMainSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    VMainSlot->SetOffsets(FMargin(60.f, 20.f, 60.f, 20.f));

    UTextBlock* Title = LSW_MakeText(WidgetTree, "CHOIX DU NIVEAU", 30, C_Gold, ETextJustify::Center);
    UVerticalBoxSlot* TitleSlot = VMain->AddChildToVerticalBox(Title);
    TitleSlot->SetHorizontalAlignment(HAlign_Center);
    TitleSlot->SetPadding(FMargin(0.f, 16.f, 0.f, 24.f));

    // ── Carousel ─────────────────────────────────────────────────────────────
    UHorizontalBox* CarouselRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* CarouselSlot = VMain->AddChildToVerticalBox(CarouselRow);
    CarouselSlot->SetHorizontalAlignment(HAlign_Center);
    CarouselSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));

    BtnPrev = WidgetTree->ConstructWidget<UButton>();
    BtnPrev->SetStyle(LSW_MakeButtonStyle(C_PanelDark, C_PanelMid));
    BtnPrev->AddChild(LSW_MakeText(WidgetTree, " < ", 24, C_Gold));
    UHorizontalBoxSlot* PrevSlot = CarouselRow->AddChildToHorizontalBox(BtnPrev);
    PrevSlot->SetVerticalAlignment(VAlign_Center);
    PrevSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    PrevSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));

    CarouselBox = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* CBoxSlot = CarouselRow->AddChildToHorizontalBox(CarouselBox);
    CBoxSlot->SetVerticalAlignment(VAlign_Center);
    CBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

    BtnNext = WidgetTree->ConstructWidget<UButton>();
    BtnNext->SetStyle(LSW_MakeButtonStyle(C_PanelDark, C_PanelMid));
    BtnNext->AddChild(LSW_MakeText(WidgetTree, " > ", 24, C_Gold));
    UHorizontalBoxSlot* NextSlot = CarouselRow->AddChildToHorizontalBox(BtnNext);
    NextSlot->SetVerticalAlignment(VAlign_Center);
    NextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    NextSlot->SetPadding(FMargin(16.f, 0.f, 0.f, 0.f));

    // ── Info niveau ───────────────────────────────────────────────────────────
    UBorder* InfoPanel = WidgetTree->ConstructWidget<UBorder>();
    InfoPanel->SetBrushColor(C_PanelDark);
    UVerticalBoxSlot* InfoSlot = VMain->AddChildToVerticalBox(InfoPanel);
    InfoSlot->SetHorizontalAlignment(HAlign_Center);
    InfoSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));

    UVerticalBox* InfoVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UBorderSlot* InfoBSlot = Cast<UBorderSlot>(InfoPanel->AddChild(InfoVBox));
    if (InfoBSlot) InfoBSlot->SetPadding(FMargin(40.f, 12.f));

    LevelNameText = LSW_MakeText(WidgetTree, "---", 22, C_White, ETextJustify::Center);
    UVerticalBoxSlot* NameSlot = InfoVBox->AddChildToVerticalBox(LevelNameText);
    NameSlot->SetHorizontalAlignment(HAlign_Center);
    NameSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

    BestScoreText = LSW_MakeText(WidgetTree, "MEILLEUR SCORE : ---", 16, C_GoldDim, ETextJustify::Center);
    InfoVBox->AddChildToVerticalBox(BestScoreText)->SetHorizontalAlignment(HAlign_Center);

    // ── Leaderboard ───────────────────────────────────────────────────────────
    UBorder* LbPanel = WidgetTree->ConstructWidget<UBorder>();
    LbPanel->SetBrushColor(C_PanelDark);
    UVerticalBoxSlot* LbPanelSlot = VMain->AddChildToVerticalBox(LbPanel);
    LbPanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    LbPanelSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));

    UVerticalBox* LbVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    if (UBorderSlot* LbBSlot = Cast<UBorderSlot>(LbPanel->AddChild(LbVBox)))
        LbBSlot->SetPadding(FMargin(24.f, 12.f));

    LbTitle = LSW_MakeText(WidgetTree, "CLASSEMENT", 16, C_Gold);
    LbVBox->AddChildToVerticalBox(LbTitle)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    LeaderboardBox = WidgetTree->ConstructWidget<UVerticalBox>();
    LbVBox->AddChildToVerticalBox(LeaderboardBox)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // ── Boutons bas ───────────────────────────────────────────────────────────
    UHorizontalBox* BtnRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* BtnRowSlot = VMain->AddChildToVerticalBox(BtnRow);
    BtnRowSlot->SetHorizontalAlignment(HAlign_Fill);
    BtnRowSlot->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));

    BtnBack = WidgetTree->ConstructWidget<UButton>();
    BtnBack->SetStyle(LSW_MakeButtonStyle(C_PanelMid, UUIHelpers::WithAlpha(C_Gold, 0.15f)));
    BtnBack->AddChild(LSW_MakeText(WidgetTree, "< RETOUR", 16, C_Grey));
    UHorizontalBoxSlot* BackHSlot = BtnRow->AddChildToHorizontalBox(BtnBack);
    BackHSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    BackHSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));

    UHorizontalBoxSlot* SpacerSlot = BtnRow->AddChildToHorizontalBox(
        WidgetTree->ConstructWidget<UHorizontalBox>());
    SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    BtnConfirm = WidgetTree->ConstructWidget<UButton>();
    BtnConfirm->SetStyle(LSW_MakeButtonStyle(
        UUIHelpers::WithAlpha(C_Red, 0.85f), C_Red));
    BtnConfirm->AddChild(LSW_MakeText(WidgetTree, "SELECTIONNER  >", 16, C_White));
    BtnRow->AddChildToHorizontalBox(BtnConfirm)->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
}

// ─────────────────────────────────────────────────────────────────────────────

void ULevelSelectWidget::PopulateLevels(const TArray<FLevelInfo>& Levels)
{
    CachedLevels = Levels;
    SelectedIndex = 0;

    if (!CarouselBox) return;

    RefreshCarousel();
    RefreshLeaderboard();
}

void ULevelSelectWidget::RefreshCarousel()
{
    if (!CarouselBox || CachedLevels.IsEmpty()) return;
    CarouselBox->ClearChildren();

    UUITheme* T = GetTheme();
    const FLinearColor C_White    = T ? T->WhiteText   : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
    const FLinearColor C_Grey     = T ? T->GreySoft    : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
    const FLinearColor C_BgPanel  = T ? T->BgPanel     : FLinearColor(0.07f, 0.06f, 0.04f, 1.f);
    const FLinearColor C_BgDeep   = T ? T->BgDeep      : FLinearColor(0.03f, 0.025f, 0.02f, 1.f);
    const FLinearColor C_CardSel  = FLinearColor(0.15f, 0.12f, 0.07f, 1.f);
    const FLinearColor C_CardUns  = FLinearColor(0.06f, 0.05f, 0.04f, 1.f);

    const int32 Num   = CachedLevels.Num();
    const int32 Range = 2;

    for (int32 Offset = -Range; Offset <= Range; ++Offset)
    {
        const int32 Idx = SelectedIndex + Offset;
        if (Idx < 0 || Idx >= Num) continue;

        const bool bSelected = (Offset == 0);
        const float W     = bSelected ? 280.f : 160.f;
        const float H     = bSelected ? 200.f : 120.f;
        const float Alpha = bSelected ? 1.f : 0.45f;

        UBorder* Card = WidgetTree->ConstructWidget<UBorder>();
        Card->SetBrushColor(bSelected ? C_CardSel : C_CardUns);

        UVerticalBox* CardVBox = WidgetTree->ConstructWidget<UVerticalBox>();
        if (UBorderSlot* BS = Cast<UBorderSlot>(Card->AddChild(CardVBox)))
            BS->SetPadding(FMargin(4.f));

        UImage* Thumb = WidgetTree->ConstructWidget<UImage>();
        if (CachedLevels[Idx].Thumbnail)
            Thumb->SetBrushFromTexture(CachedLevels[Idx].Thumbnail);
        else
            Thumb->SetColorAndOpacity(UUIHelpers::WithAlpha(C_BgPanel, Alpha));

        USizeBox* ThumbSB = WidgetTree->ConstructWidget<USizeBox>();
        ThumbSB->SetWidthOverride(W - 8.f);
        ThumbSB->SetHeightOverride(H - 30.f);
        ThumbSB->AddChild(Thumb);
        CardVBox->AddChildToVerticalBox(ThumbSB)->SetHorizontalAlignment(HAlign_Center);

        UTextBlock* CardName = LSW_MakeText(WidgetTree,
            CachedLevels[Idx].DisplayName.ToString(),
            bSelected ? 13 : 10,
            bSelected ? C_White : C_Grey,
            ETextJustify::Center);
        UVerticalBoxSlot* NameSlot = CardVBox->AddChildToVerticalBox(CardName);
        NameSlot->SetHorizontalAlignment(HAlign_Center);
        NameSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));

        UHorizontalBoxSlot* HSlot = CarouselBox->AddChildToHorizontalBox(Card);
        HSlot->SetVerticalAlignment(VAlign_Center);
        HSlot->SetPadding(FMargin(bSelected ? 8.f : 4.f, 0.f));
    }

    if (LevelNameText && CachedLevels.IsValidIndex(SelectedIndex))
        LevelNameText->SetText(CachedLevels[SelectedIndex].DisplayName);

    if (BestScoreText && CachedLevels.IsValidIndex(SelectedIndex))
    {
        FString SlotName = FString::Printf(TEXT("Leaderboard_%s"),
            *CachedLevels[SelectedIndex].MapName.ToString());
        ULeaderboardSaveGame* Save = Cast<ULeaderboardSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SlotName, 0));
        BestScoreText->SetText(FText::FromString(
            (Save && Save->Scores.Num() > 0)
            ? FString::Printf(TEXT("MEILLEUR SCORE : %d"), Save->Scores[0].Score)
            : FString(TEXT("MEILLEUR SCORE : ---"))));
    }
}

void ULevelSelectWidget::RefreshLeaderboard()
{
    if (!LeaderboardBox || !CachedLevels.IsValidIndex(SelectedIndex)) return;
    LeaderboardBox->ClearChildren();

    UUITheme* T = GetTheme();
    const FLinearColor C_Gold    = T ? T->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
    const FLinearColor C_GoldDim = T ? T->GoldDim     : FLinearColor(0.55f, 0.45f, 0.20f, 1.f);
    const FLinearColor C_White   = T ? T->WhiteText   : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
    const FLinearColor C_Grey    = T ? T->GreySoft    : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);

    FString SlotName = FString::Printf(TEXT("Leaderboard_%s"),
        *CachedLevels[SelectedIndex].MapName.ToString());
    ULeaderboardSaveGame* Save = Cast<ULeaderboardSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!Save || Save->Scores.IsEmpty())
    {
        LeaderboardBox->AddChildToVerticalBox(
            LSW_MakeText(WidgetTree, TEXT("Aucun score enregistre"), 14, C_Grey));
        return;
    }

    UHorizontalBox* Headers = WidgetTree->ConstructWidget<UHorizontalBox>();
    LeaderboardBox->AddChildToVerticalBox(Headers)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

    auto AddHeader = [&](const FString& Label, float Fill) {
        UHorizontalBoxSlot* S = Headers->AddChildToHorizontalBox(
            LSW_MakeText(WidgetTree, Label, 12, C_GoldDim));
        FSlateChildSize SZ(ESlateSizeRule::Fill); SZ.Value = Fill; S->SetSize(SZ);
    };
    AddHeader("#", 0.5f); AddHeader("SCORE", 2.f); AddHeader("KILLS", 1.f);
    AddHeader("COMBO", 1.f); AddHeader("DATE", 2.f);

    const int32 MaxShow = FMath::Min(Save->Scores.Num(), 5);
    for (int32 i = 0; i < MaxShow; ++i)
    {
        const FScoreEntry& E = Save->Scores[i];
        const FLinearColor RowColor = (i == 0) ? C_Gold : C_White;

        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
        LeaderboardBox->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 2.f));

        auto AddCell = [&](const FString& Val, float Fill) {
            UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(
                LSW_MakeText(WidgetTree, Val, 13, RowColor));
            FSlateChildSize SZ(ESlateSizeRule::Fill); SZ.Value = Fill; S->SetSize(SZ);
        };
        AddCell(FString::Printf(TEXT("#%d"), i + 1), 0.5f);
        AddCell(FString::Printf(TEXT("%d"),  E.Score), 2.f);
        AddCell(FString::Printf(TEXT("%d"),  E.Kills), 1.f);
        AddCell(FString::Printf(TEXT("x%d"), E.BestCombo), 1.f);
        AddCell(E.Date, 2.f);
    }
}

void ULevelSelectWidget::SelectLevel(int32 Index)
{
    if (!CachedLevels.IsValidIndex(Index)) return;
    SelectedIndex = Index;
    RefreshCarousel();
    RefreshLeaderboard();
}

// ─────────────────────────────────────────────────────────────────────────────

void ULevelSelectWidget::HandlePrev()
{
    UE_LOG(LogTemp, Warning, TEXT("[LevelSelect] HandlePrev — SelectedIndex=%d Num=%d BtnPrev=%s"),
        SelectedIndex, CachedLevels.Num(), BtnPrev ? TEXT("OK") : TEXT("null"));
    if (SelectedIndex > 0)
        SelectLevel(SelectedIndex - 1);
}

void ULevelSelectWidget::HandleNext()
{
    UE_LOG(LogTemp, Warning, TEXT("[LevelSelect] HandleNext — SelectedIndex=%d Num=%d BtnNext=%s"),
        SelectedIndex, CachedLevels.Num(), BtnNext ? TEXT("OK") : TEXT("null"));
    if (SelectedIndex < CachedLevels.Num() - 1)
        SelectLevel(SelectedIndex + 1);
}

void ULevelSelectWidget::HandleConfirm()
{
    PlayClickSound();
    if (CachedLevels.IsValidIndex(SelectedIndex))
        OnLevelChosen.Broadcast(CachedLevels[SelectedIndex]);
}

void ULevelSelectWidget::HandleBack()
{
    PlayClickSound();
    OnBackClicked.Broadcast();
}
