// Copyright RevenantOps. All Rights Reserved.
#include "UI/CharacterSelectWidget.h"
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
#include "Blueprint/WidgetTree.h"

// ─── Couleurs (même palette que LevelSelectWidget) ────────────────────────────
static const FLinearColor CC_BgDark    (0.04f, 0.03f, 0.02f, 0.97f);
static const FLinearColor CC_PanelDark (0.07f, 0.06f, 0.04f, 1.f);
static const FLinearColor CC_PanelMid  (0.10f, 0.09f, 0.07f, 1.f);
static const FLinearColor CC_Gold      (0.85f, 0.70f, 0.30f, 1.f);
static const FLinearColor CC_GoldDim   (0.55f, 0.45f, 0.20f, 1.f);
static const FLinearColor CC_White     (1.f,   1.f,   1.f,   1.f);
static const FLinearColor CC_Grey      (0.5f,  0.5f,  0.5f,  1.f);
static const FLinearColor CC_Red       (0.75f, 0.15f, 0.10f, 1.f);
static const FLinearColor CC_CardSel   (0.15f, 0.12f, 0.07f, 1.f);
static const FLinearColor CC_CardUnsel (0.06f, 0.05f, 0.04f, 1.f);

static FSlateBrush CCMakeBrush(FLinearColor Color)
{
    FSlateBrush B; B.TintColor = FSlateColor(Color); return B;
}

static FButtonStyle CCMakeButtonStyle(FLinearColor Normal, FLinearColor Hover)
{
    FButtonStyle S;
    S.SetNormal(CCMakeBrush(Normal));
    S.SetHovered(CCMakeBrush(Hover));
    S.SetPressed(CCMakeBrush(Normal * 0.7f));
    return S;
}

static UTextBlock* CCMakeText(UWidgetTree* WT, const FString& Str, int32 Size,
                               FLinearColor Color, ETextJustify::Type Justify = ETextJustify::Left)
{
    UTextBlock* T = WT->ConstructWidget<UTextBlock>();
    T->SetText(FText::FromString(Str));
    FSlateFontInfo F = T->GetFont(); F.Size = Size; T->SetFont(F);
    T->SetColorAndOpacity(FSlateColor(Color));
    T->SetJustification(Justify);
    return T;
}

// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> UCharacterSelectWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt)
        BuildDefaultUI();
    return Super::RebuildWidget();
}

void UCharacterSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (BtnBack)    BtnBack->OnClicked.AddDynamic(this,    &UCharacterSelectWidget::HandleBack);
    if (BtnPrev)    BtnPrev->OnClicked.AddDynamic(this,    &UCharacterSelectWidget::HandlePrev);
    if (BtnNext)    BtnNext->OnClicked.AddDynamic(this,    &UCharacterSelectWidget::HandleNext);
    if (BtnConfirm) BtnConfirm->OnClicked.AddDynamic(this, &UCharacterSelectWidget::HandleConfirm);

    if (!CachedCharacters.IsEmpty())
        PopulateCharacters(CachedCharacters);
}

// ─────────────────────────────────────────────────────────────────────────────

void UCharacterSelectWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    // ── Root ─────────────────────────────────────────────────────────────────
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(CC_BgDark);
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Bande rouge en haut
    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(CC_Red);
    UCanvasPanelSlot* TopBarSlot = Root->AddChildToCanvas(TopBar);
    TopBarSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopBarSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 4.f));
    TopBarSlot->SetAutoSize(true);

    // ── Layout principal ──────────────────────────────────────────────────────
    UVerticalBox* VMain = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VSlot = Root->AddChildToCanvas(VMain);
    VSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    VSlot->SetOffsets(FMargin(60.f, 20.f, 60.f, 20.f));

    // Titre
    UTextBlock* Title = CCMakeText(WidgetTree, TEXT("SELECTION DU PERSONNAGE"), 30, CC_Gold, ETextJustify::Center);
    UVerticalBoxSlot* TitleSlot = VMain->AddChildToVerticalBox(Title);
    TitleSlot->SetHorizontalAlignment(HAlign_Center);
    TitleSlot->SetPadding(FMargin(0.f, 16.f, 0.f, 24.f));

    // ── Zone centrale : portrait gauche + carousel + info droite ──────────────
    UHorizontalBox* MainRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* MainRowSlot = VMain->AddChildToVerticalBox(MainRow);
    MainRowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    MainRowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));

    // -- Portrait gauche ------
    UBorder* PortraitPanel = WidgetTree->ConstructWidget<UBorder>();
    PortraitPanel->SetBrushColor(CC_PanelDark);
    UHorizontalBoxSlot* PortraitHSlot = MainRow->AddChildToHorizontalBox(PortraitPanel);
    PortraitHSlot->SetVerticalAlignment(VAlign_Fill);
    FSlateChildSize PortraitSize(ESlateSizeRule::Fill); PortraitSize.Value = 1.2f;
    PortraitHSlot->SetSize(PortraitSize);
    PortraitHSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

    PortraitImage = WidgetTree->ConstructWidget<UImage>();
    PortraitImage->SetColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f, 1.f));
    UBorderSlot* PortraitBSlot = Cast<UBorderSlot>(PortraitPanel->AddChild(PortraitImage));
    if (PortraitBSlot)
    {
        PortraitBSlot->SetHorizontalAlignment(HAlign_Fill);
        PortraitBSlot->SetVerticalAlignment(VAlign_Fill);
    }

    // -- Centre : carousel ----
    UVerticalBox* CenterVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UHorizontalBoxSlot* CenterHSlot = MainRow->AddChildToHorizontalBox(CenterVBox);
    FSlateChildSize CenterSize(ESlateSizeRule::Fill); CenterSize.Value = 2.f;
    CenterHSlot->SetSize(CenterSize);
    CenterHSlot->SetVerticalAlignment(VAlign_Center);

    // Carousel row : < [cartes] >
    UHorizontalBox* CarouselRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* CarouselVSlot = CenterVBox->AddChildToVerticalBox(CarouselRow);
    CarouselVSlot->SetHorizontalAlignment(HAlign_Center);
    CarouselVSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));

    BtnPrev = WidgetTree->ConstructWidget<UButton>();
    BtnPrev->SetStyle(CCMakeButtonStyle(CC_PanelDark, CC_PanelMid));
    BtnPrev->AddChild(CCMakeText(WidgetTree, TEXT(" < "), 22, CC_Gold));
    UHorizontalBoxSlot* PrevHS = CarouselRow->AddChildToHorizontalBox(BtnPrev);
    PrevHS->SetVerticalAlignment(VAlign_Center);
    PrevHS->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

    CarouselBox = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* CarouselHS = CarouselRow->AddChildToHorizontalBox(CarouselBox);
    CarouselHS->SetVerticalAlignment(VAlign_Center);

    BtnNext = WidgetTree->ConstructWidget<UButton>();
    BtnNext->SetStyle(CCMakeButtonStyle(CC_PanelDark, CC_PanelMid));
    BtnNext->AddChild(CCMakeText(WidgetTree, TEXT(" > "), 22, CC_Gold));
    UHorizontalBoxSlot* NextHS = CarouselRow->AddChildToHorizontalBox(BtnNext);
    NextHS->SetVerticalAlignment(VAlign_Center);
    NextHS->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));

    // Nom du perso
    CharNameText = CCMakeText(WidgetTree, TEXT("---"), 24, CC_White, ETextJustify::Center);
    UVerticalBoxSlot* NameVSlot = CenterVBox->AddChildToVerticalBox(CharNameText);
    NameVSlot->SetHorizontalAlignment(HAlign_Center);
    NameVSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // -- Info droite ----------
    UBorder* InfoPanel = WidgetTree->ConstructWidget<UBorder>();
    InfoPanel->SetBrushColor(CC_PanelDark);
    UHorizontalBoxSlot* InfoHSlot = MainRow->AddChildToHorizontalBox(InfoPanel);
    FSlateChildSize InfoSize(ESlateSizeRule::Fill); InfoSize.Value = 1.2f;
    InfoHSlot->SetSize(InfoSize);
    InfoHSlot->SetVerticalAlignment(VAlign_Fill);
    InfoHSlot->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));

    UVerticalBox* InfoVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UBorderSlot* InfoBSlot = Cast<UBorderSlot>(InfoPanel->AddChild(InfoVBox));
    if (InfoBSlot) InfoBSlot->SetPadding(FMargin(16.f, 16.f));

    UTextBlock* InfoTitle = CCMakeText(WidgetTree, TEXT("MERCENAIRE"), 14, CC_GoldDim);
    UVerticalBoxSlot* InfoTitleSlot = InfoVBox->AddChildToVerticalBox(InfoTitle);
    InfoTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));

    // Ligne décorative
    UBorder* Sep = WidgetTree->ConstructWidget<UBorder>();
    Sep->SetBrushColor(CC_Red);
    UVerticalBoxSlot* SepSlot = InfoVBox->AddChildToVerticalBox(Sep);
    SepSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
    UBorderSlot* SepBSlot = Cast<UBorderSlot>(Sep->AddChild(WidgetTree->ConstructWidget<UTextBlock>()));
    // hauteur 2px via padding
    if (SepBSlot) SepBSlot->SetPadding(FMargin(0.f, 1.f));

    UTextBlock* InfoHint = CCMakeText(WidgetTree,
        TEXT("Utilisez < > pour\nnaviguer entre\nles personnages."),
        13, CC_Grey);
    InfoHint->SetAutoWrapText(true);
    UVerticalBoxSlot* HintSlot = InfoVBox->AddChildToVerticalBox(InfoHint);
    HintSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 0.f));

    // ── Boutons bas ───────────────────────────────────────────────────────────
    UHorizontalBox* BtnRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* BtnRowSlot = VMain->AddChildToVerticalBox(BtnRow);
    BtnRowSlot->SetHorizontalAlignment(HAlign_Fill);

    BtnBack = WidgetTree->ConstructWidget<UButton>();
    BtnBack->SetStyle(CCMakeButtonStyle(CC_PanelMid, FLinearColor(0.18f, 0.14f, 0.10f, 1.f)));
    BtnBack->AddChild(CCMakeText(WidgetTree, TEXT("< RETOUR"), 16, CC_Grey));
    UHorizontalBoxSlot* BackHS = BtnRow->AddChildToHorizontalBox(BtnBack);
    BackHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    BackHS->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));

    UHorizontalBox* Spacer = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* SpacerHS = BtnRow->AddChildToHorizontalBox(Spacer);
    SpacerHS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    BtnConfirm = WidgetTree->ConstructWidget<UButton>();
    BtnConfirm->SetStyle(CCMakeButtonStyle(CC_Red, FLinearColor(0.9f, 0.2f, 0.15f, 1.f)));
    BtnConfirm->AddChild(CCMakeText(WidgetTree, TEXT("SELECTIONNER  >"), 16, CC_White));
    UHorizontalBoxSlot* ConfirmHS = BtnRow->AddChildToHorizontalBox(BtnConfirm);
    ConfirmHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
}

// ─────────────────────────────────────────────────────────────────────────────

void UCharacterSelectWidget::PopulateCharacters(const TArray<FCharacterInfo>& Characters)
{
    CachedCharacters = Characters;
    SelectedIndex = 0;
    if (!CarouselBox) return;
    RefreshCarousel();
    RefreshInfo();
}

void UCharacterSelectWidget::RefreshCarousel()
{
    if (!CarouselBox || CachedCharacters.IsEmpty()) return;
    CarouselBox->ClearChildren();

    const int32 Num = CachedCharacters.Num();
    const int32 Range = 2;

    for (int32 Offset = -Range; Offset <= Range; ++Offset)
    {
        int32 Idx = SelectedIndex + Offset;
        if (Idx < 0 || Idx >= Num) continue;

        bool bSelected = (Offset == 0);
        float W = bSelected ? 160.f : 90.f;
        float H = bSelected ? 220.f : 130.f;

        UBorder* Card = WidgetTree->ConstructWidget<UBorder>();
        Card->SetBrushColor(bSelected ? CC_CardSel : CC_CardUnsel);

        UVerticalBox* CardVBox = WidgetTree->ConstructWidget<UVerticalBox>();
        UBorderSlot* CardBSlot = Cast<UBorderSlot>(Card->AddChild(CardVBox));
        if (CardBSlot) CardBSlot->SetPadding(FMargin(3.f));

        UImage* Thumb = WidgetTree->ConstructWidget<UImage>();
        if (CachedCharacters[Idx].Thumbnail)
            Thumb->SetBrushFromTexture(CachedCharacters[Idx].Thumbnail);
        else
            Thumb->SetColorAndOpacity(FLinearColor(0.18f, 0.15f, 0.12f, bSelected ? 1.f : 0.5f));

        USizeBox* ThumbSB = WidgetTree->ConstructWidget<USizeBox>();
        ThumbSB->SetWidthOverride(W - 6.f);
        ThumbSB->SetHeightOverride(H - 28.f);
        ThumbSB->AddChild(Thumb);
        UVerticalBoxSlot* ThumbVSlot = CardVBox->AddChildToVerticalBox(ThumbSB);
        ThumbVSlot->SetHorizontalAlignment(HAlign_Center);

        UTextBlock* CardName = CCMakeText(WidgetTree,
            CachedCharacters[Idx].DisplayName.ToString(),
            bSelected ? 12 : 9,
            bSelected ? CC_White : CC_Grey,
            ETextJustify::Center);
        UVerticalBoxSlot* CardNameSlot = CardVBox->AddChildToVerticalBox(CardName);
        CardNameSlot->SetHorizontalAlignment(HAlign_Center);
        CardNameSlot->SetPadding(FMargin(0.f, 3.f, 0.f, 0.f));

        UHorizontalBoxSlot* HSlot = CarouselBox->AddChildToHorizontalBox(Card);
        HSlot->SetVerticalAlignment(VAlign_Center);
        HSlot->SetPadding(FMargin(bSelected ? 6.f : 3.f, 0.f));
    }
}

void UCharacterSelectWidget::RefreshInfo()
{
    if (CharNameText && CachedCharacters.IsValidIndex(SelectedIndex))
        CharNameText->SetText(CachedCharacters[SelectedIndex].DisplayName);

    if (PortraitImage && CachedCharacters.IsValidIndex(SelectedIndex))
    {
        if (CachedCharacters[SelectedIndex].Thumbnail)
            PortraitImage->SetBrushFromTexture(CachedCharacters[SelectedIndex].Thumbnail);
        else
            PortraitImage->SetColorAndOpacity(FLinearColor(0.2f, 0.17f, 0.13f, 1.f));
    }
}

void UCharacterSelectWidget::SelectCharacter(int32 Index)
{
    if (!CachedCharacters.IsValidIndex(Index)) return;
    SelectedIndex = Index;
    RefreshCarousel();
    RefreshInfo();
}

void UCharacterSelectWidget::HandlePrev()
{
    if (SelectedIndex > 0) SelectCharacter(SelectedIndex - 1);
}

void UCharacterSelectWidget::HandleNext()
{
    if (SelectedIndex < CachedCharacters.Num() - 1) SelectCharacter(SelectedIndex + 1);
}

void UCharacterSelectWidget::HandleConfirm()
{
    if (CachedCharacters.IsValidIndex(SelectedIndex))
        OnCharacterChosen.Broadcast(CachedCharacters[SelectedIndex]);
}

void UCharacterSelectWidget::HandleBack()
{
    OnBackClicked.Broadcast();
}
