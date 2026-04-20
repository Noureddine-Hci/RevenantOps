// Copyright RevenantOps. All Rights Reserved.
#include "UI/LevelSelectWidget.h"
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

// ─── Couleurs militaires ──────────────────────────────────────────────────────
static const FLinearColor C_BgDark      (0.04f, 0.03f, 0.02f, 0.97f);
static const FLinearColor C_PanelDark   (0.07f, 0.06f, 0.04f, 1.f);
static const FLinearColor C_PanelMid    (0.10f, 0.09f, 0.07f, 1.f);
static const FLinearColor C_Gold        (0.85f, 0.70f, 0.30f, 1.f);
static const FLinearColor C_GoldDim     (0.55f, 0.45f, 0.20f, 1.f);
static const FLinearColor C_White       (1.f,   1.f,   1.f,   1.f);
static const FLinearColor C_Grey        (0.5f,  0.5f,  0.5f,  1.f);
static const FLinearColor C_Red         (0.75f, 0.15f, 0.10f, 1.f);
static const FLinearColor C_CardSel     (0.15f, 0.12f, 0.07f, 1.f);
static const FLinearColor C_CardUnsel   (0.06f, 0.05f, 0.04f, 1.f);

// ─── Helpers ──────────────────────────────────────────────────────────────────
static FSlateBrush MakeBrush(FLinearColor Color)
{
    FSlateBrush B;
    B.TintColor = FSlateColor(Color);
    return B;
}

static FButtonStyle MakeButtonStyle(FLinearColor Normal, FLinearColor Hover)
{
    FButtonStyle S;
    S.SetNormal(MakeBrush(Normal));
    S.SetHovered(MakeBrush(Hover));
    S.SetPressed(MakeBrush(Normal * 0.7f));
    return S;
}

static UTextBlock* MakeText(UWidgetTree* WT, const FString& Str, int32 Size,
                             FLinearColor Color, ETextJustify::Type Justify = ETextJustify::Left)
{
    UTextBlock* T = WT->ConstructWidget<UTextBlock>();
    T->SetText(FText::FromString(Str));
    FSlateFontInfo F = T->GetFont();
    F.Size = Size;
    T->SetFont(F);
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
    if (BtnBack)    BtnBack->OnClicked.AddDynamic(this,    &ULevelSelectWidget::HandleBack);
    if (BtnPrev)    BtnPrev->OnClicked.AddDynamic(this,    &ULevelSelectWidget::HandlePrev);
    if (BtnNext)    BtnNext->OnClicked.AddDynamic(this,    &ULevelSelectWidget::HandleNext);
    if (BtnConfirm) BtnConfirm->OnClicked.AddDynamic(this, &ULevelSelectWidget::HandleConfirm);

    if (!CachedLevels.IsEmpty())
        PopulateLevels(CachedLevels);
}

// ─────────────────────────────────────────────────────────────────────────────

void ULevelSelectWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    // ── Root canvas ──────────────────────────────────────────────────────────
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    // Fond plein écran
    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(C_BgDark);
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Bande décorative haut (accent rouge)
    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(C_Red);
    UCanvasPanelSlot* TopBarSlot = Root->AddChildToCanvas(TopBar);
    TopBarSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopBarSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 4.f));
    TopBarSlot->SetAutoSize(true);

    // ── Layout vertical principal ─────────────────────────────────────────────
    UVerticalBox* VMain = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VMainSlot = Root->AddChildToCanvas(VMain);
    VMainSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    VMainSlot->SetOffsets(FMargin(60.f, 20.f, 60.f, 20.f));

    // ── Titre ─────────────────────────────────────────────────────────────────
    UTextBlock* Title = MakeText(WidgetTree, "CHOIX DU NIVEAU", 30, C_Gold, ETextJustify::Center);
    UVerticalBoxSlot* TitleSlot = VMain->AddChildToVerticalBox(Title);
    TitleSlot->SetHorizontalAlignment(HAlign_Center);
    TitleSlot->SetPadding(FMargin(0.f, 16.f, 0.f, 24.f));

    // ── Carousel ─────────────────────────────────────────────────────────────
    // [ < ]  [Card][Card][CARD CENTRAL][Card][Card]  [ > ]
    UHorizontalBox* CarouselRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* CarouselSlot = VMain->AddChildToVerticalBox(CarouselRow);
    CarouselSlot->SetHorizontalAlignment(HAlign_Center);
    CarouselSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));

    // Bouton précédent
    BtnPrev = WidgetTree->ConstructWidget<UButton>();
    BtnPrev->SetStyle(MakeButtonStyle(C_PanelDark, C_PanelMid));
    UTextBlock* PrevLbl = MakeText(WidgetTree, " < ", 24, C_Gold);
    BtnPrev->AddChild(PrevLbl);
    UHorizontalBoxSlot* PrevSlot = CarouselRow->AddChildToHorizontalBox(BtnPrev);
    PrevSlot->SetVerticalAlignment(VAlign_Center);
    PrevSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));

    // Conteneur des cartes
    CarouselBox = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* CBoxSlot = CarouselRow->AddChildToHorizontalBox(CarouselBox);
    CBoxSlot->SetVerticalAlignment(VAlign_Center);

    // Bouton suivant
    BtnNext = WidgetTree->ConstructWidget<UButton>();
    BtnNext->SetStyle(MakeButtonStyle(C_PanelDark, C_PanelMid));
    UTextBlock* NextLbl = MakeText(WidgetTree, " > ", 24, C_Gold);
    BtnNext->AddChild(NextLbl);
    UHorizontalBoxSlot* NextSlot = CarouselRow->AddChildToHorizontalBox(BtnNext);
    NextSlot->SetVerticalAlignment(VAlign_Center);
    NextSlot->SetPadding(FMargin(16.f, 0.f, 0.f, 0.f));

    // ── Nom du niveau + meilleur score ────────────────────────────────────────
    UBorder* InfoPanel = WidgetTree->ConstructWidget<UBorder>();
    InfoPanel->SetBrushColor(C_PanelDark);
    UVerticalBoxSlot* InfoSlot = VMain->AddChildToVerticalBox(InfoPanel);
    InfoSlot->SetHorizontalAlignment(HAlign_Center);
    InfoSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));

    UVerticalBox* InfoVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UBorderSlot* InfoBSlot = Cast<UBorderSlot>(InfoPanel->AddChild(InfoVBox));
    if (InfoBSlot) InfoBSlot->SetPadding(FMargin(40.f, 12.f));

    LevelNameText = MakeText(WidgetTree, "---", 22, C_White, ETextJustify::Center);
    UVerticalBoxSlot* NameSlot = InfoVBox->AddChildToVerticalBox(LevelNameText);
    NameSlot->SetHorizontalAlignment(HAlign_Center);
    NameSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

    BestScoreText = MakeText(WidgetTree, "MEILLEUR SCORE : ---", 16, C_GoldDim, ETextJustify::Center);
    UVerticalBoxSlot* BestSlot = InfoVBox->AddChildToVerticalBox(BestScoreText);
    BestSlot->SetHorizontalAlignment(HAlign_Center);

    // ── Leaderboard ───────────────────────────────────────────────────────────
    UBorder* LbPanel = WidgetTree->ConstructWidget<UBorder>();
    LbPanel->SetBrushColor(C_PanelDark);
    UVerticalBoxSlot* LbPanelSlot = VMain->AddChildToVerticalBox(LbPanel);
    LbPanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    LbPanelSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));

    UVerticalBox* LbVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UBorderSlot* LbBSlot = Cast<UBorderSlot>(LbPanel->AddChild(LbVBox));
    if (LbBSlot) LbBSlot->SetPadding(FMargin(24.f, 12.f));

    LbTitle = MakeText(WidgetTree, "CLASSEMENT", 16, C_Gold);
    UVerticalBoxSlot* LbTitleSlot = LbVBox->AddChildToVerticalBox(LbTitle);
    LbTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    LeaderboardBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UVerticalBoxSlot* LeadSlot = LbVBox->AddChildToVerticalBox(LeaderboardBox);
    LeadSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // ── Boutons bas ───────────────────────────────────────────────────────────
    UHorizontalBox* BtnRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* BtnRowSlot = VMain->AddChildToVerticalBox(BtnRow);
    BtnRowSlot->SetHorizontalAlignment(HAlign_Fill);
    BtnRowSlot->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));

    // Retour
    BtnBack = WidgetTree->ConstructWidget<UButton>();
    BtnBack->SetStyle(MakeButtonStyle(C_PanelMid, FLinearColor(0.18f, 0.14f, 0.10f, 1.f)));
    UTextBlock* BackLbl = MakeText(WidgetTree, "< RETOUR", 16, C_Grey);
    BtnBack->AddChild(BackLbl);
    UHorizontalBoxSlot* BackHSlot = BtnRow->AddChildToHorizontalBox(BtnBack);
    BackHSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    BackHSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));

    // Spacer
    UHorizontalBox* Spacer = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* SpacerSlot = BtnRow->AddChildToHorizontalBox(Spacer);
    SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // Confirmer
    BtnConfirm = WidgetTree->ConstructWidget<UButton>();
    BtnConfirm->SetStyle(MakeButtonStyle(C_Red, FLinearColor(0.9f, 0.2f, 0.15f, 1.f)));
    UTextBlock* ConfirmLbl = MakeText(WidgetTree, "SELECTIONNER  >", 16, C_White);
    BtnConfirm->AddChild(ConfirmLbl);
    UHorizontalBoxSlot* ConfirmHSlot = BtnRow->AddChildToHorizontalBox(BtnConfirm);
    ConfirmHSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
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

    // Affiche jusqu'à 5 cartes : 2 avant, centrale, 2 après
    const int32 Num = CachedLevels.Num();
    const int32 Range = 2;

    for (int32 Offset = -Range; Offset <= Range; ++Offset)
    {
        int32 Idx = SelectedIndex + Offset;
        if (Idx < 0 || Idx >= Num) continue;

        bool bSelected = (Offset == 0);

        // Taille : carte centrale plus grande
        float W = bSelected ? 280.f : 160.f;
        float H = bSelected ? 200.f : 120.f;
        float Alpha = bSelected ? 1.f : 0.45f;

        UBorder* Card = WidgetTree->ConstructWidget<UBorder>();
        Card->SetBrushColor(bSelected ? C_CardSel : C_CardUnsel);

        UVerticalBox* CardVBox = WidgetTree->ConstructWidget<UVerticalBox>();
        UBorderSlot* CardBSlot = Cast<UBorderSlot>(Card->AddChild(CardVBox));
        if (CardBSlot) CardBSlot->SetPadding(FMargin(4.f));

        // Thumbnail ou placeholder gris
        UImage* Thumb = WidgetTree->ConstructWidget<UImage>();
        if (CachedLevels[Idx].Thumbnail)
            Thumb->SetBrushFromTexture(CachedLevels[Idx].Thumbnail);
        else
            Thumb->SetColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, Alpha));

        USizeBox* ThumbSB = WidgetTree->ConstructWidget<USizeBox>();
        ThumbSB->SetWidthOverride(W - 8.f);
        ThumbSB->SetHeightOverride(H - 30.f);
        ThumbSB->AddChild(Thumb);
        UVerticalBoxSlot* ThumbSlot = CardVBox->AddChildToVerticalBox(ThumbSB);
        ThumbSlot->SetHorizontalAlignment(HAlign_Center);

        // Nom sous la carte
        UTextBlock* CardName = MakeText(WidgetTree,
            CachedLevels[Idx].DisplayName.ToString(),
            bSelected ? 13 : 10,
            bSelected ? C_White : C_Grey,
            ETextJustify::Center);
        UVerticalBoxSlot* CardNameSlot = CardVBox->AddChildToVerticalBox(CardName);
        CardNameSlot->SetHorizontalAlignment(HAlign_Center);
        CardNameSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));

        // Bordure rouge sur la carte sélectionnée
        if (bSelected)
        {
            FSlateBrush SelBrush = MakeBrush(C_Red);
            Card->SetBrushColor(C_CardSel);
        }

        UHorizontalBoxSlot* HSlot = CarouselBox->AddChildToHorizontalBox(Card);
        HSlot->SetVerticalAlignment(VAlign_Center);
        HSlot->SetPadding(FMargin(bSelected ? 8.f : 4.f, 0.f));
    }

    // Mise à jour nom + meilleur score
    if (LevelNameText && CachedLevels.IsValidIndex(SelectedIndex))
        LevelNameText->SetText(CachedLevels[SelectedIndex].DisplayName);

    if (BestScoreText && CachedLevels.IsValidIndex(SelectedIndex))
    {
        FString SlotName = FString::Printf(TEXT("Leaderboard_%s"),
            *CachedLevels[SelectedIndex].MapName.ToString());
        ULeaderboardSaveGame* Save = Cast<ULeaderboardSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SlotName, 0));
        if (Save && Save->Scores.Num() > 0)
            BestScoreText->SetText(FText::FromString(
                FString::Printf(TEXT("MEILLEUR SCORE : %d"), Save->Scores[0].Score)));
        else
            BestScoreText->SetText(FText::FromString(TEXT("MEILLEUR SCORE : ---")));
    }
}

void ULevelSelectWidget::RefreshLeaderboard()
{
    if (!LeaderboardBox || !CachedLevels.IsValidIndex(SelectedIndex)) return;
    LeaderboardBox->ClearChildren();

    FString SlotName = FString::Printf(TEXT("Leaderboard_%s"),
        *CachedLevels[SelectedIndex].MapName.ToString());

    ULeaderboardSaveGame* Save = Cast<ULeaderboardSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!Save || Save->Scores.IsEmpty())
    {
        UTextBlock* Empty = MakeText(WidgetTree, TEXT("Aucun score enregistre"), 14, C_Grey);
        LeaderboardBox->AddChildToVerticalBox(Empty);
        return;
    }

    // En-têtes
    UHorizontalBox* Headers = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* HeaderSlot = LeaderboardBox->AddChildToVerticalBox(Headers);
    HeaderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

    auto AddHeader = [&](const FString& Label, float Fill)
    {
        UTextBlock* T = MakeText(WidgetTree, Label, 12, C_GoldDim);
        UHorizontalBoxSlot* S = Headers->AddChildToHorizontalBox(T);
        FSlateChildSize SZ(ESlateSizeRule::Fill); SZ.Value = Fill;
        S->SetSize(SZ);
    };
    AddHeader("#",      0.5f);
    AddHeader("SCORE",  2.f);
    AddHeader("KILLS",  1.f);
    AddHeader("COMBO",  1.f);
    AddHeader("DATE",   2.f);

    // Lignes
    const int32 MaxShow = FMath::Min(Save->Scores.Num(), 5);
    for (int32 i = 0; i < MaxShow; ++i)
    {
        const FScoreEntry& E = Save->Scores[i];
        FLinearColor RowColor = (i == 0) ? C_Gold : C_White;

        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
        UVerticalBoxSlot* RowSlot = LeaderboardBox->AddChildToVerticalBox(Row);
        RowSlot->SetPadding(FMargin(0.f, 2.f));

        auto AddCell = [&](const FString& Val, float Fill)
        {
            UTextBlock* T = MakeText(WidgetTree, Val, 13, RowColor);
            UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(T);
            FSlateChildSize SZ(ESlateSizeRule::Fill); SZ.Value = Fill;
            S->SetSize(SZ);
        };
        AddCell(FString::Printf(TEXT("#%d"), i + 1),         0.5f);
        AddCell(FString::Printf(TEXT("%d"), E.Score),         2.f);
        AddCell(FString::Printf(TEXT("%d"), E.Kills),         1.f);
        AddCell(FString::Printf(TEXT("x%d"), E.BestCombo),    1.f);
        AddCell(E.Date,                                        2.f);
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
    if (SelectedIndex > 0)
        SelectLevel(SelectedIndex - 1);
}

void ULevelSelectWidget::HandleNext()
{
    if (SelectedIndex < CachedLevels.Num() - 1)
        SelectLevel(SelectedIndex + 1);
}

void ULevelSelectWidget::HandleConfirm()
{
    if (CachedLevels.IsValidIndex(SelectedIndex))
        OnLevelChosen.Broadcast(CachedLevels[SelectedIndex]);
}

void ULevelSelectWidget::HandleBack()
{
    OnBackClicked.Broadcast();
}
