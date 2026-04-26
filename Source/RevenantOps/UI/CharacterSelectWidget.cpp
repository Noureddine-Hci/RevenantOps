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
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UI/CharacterPreviewActor.h"

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

    // Spawn du preview actor loin du gameplay (Z=50000)
    if (UWorld* W = GetWorld())
    {
        TSubclassOf<ACharacterPreviewActor> ClassToSpawn = PreviewActorClass
            ? PreviewActorClass
            : TSubclassOf<ACharacterPreviewActor>(ACharacterPreviewActor::StaticClass());
        FActorSpawnParameters SP;
        SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        PreviewActor = W->SpawnActor<ACharacterPreviewActor>(
            ClassToSpawn, FVector(0.f, 0.f, 50000.f), FRotator::ZeroRotator, SP);
    }

    if (!CachedCharacters.IsEmpty())
        PopulateCharacters(CachedCharacters);
}

void UCharacterSelectWidget::NativeDestruct()
{
    if (PreviewActor) { PreviewActor->Destroy(); PreviewActor = nullptr; }
    Super::NativeDestruct();
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
    Bg->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Bande rouge en haut
    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(CC_Red);
    TopBar->SetVisibility(ESlateVisibility::HitTestInvisible);
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
    PortraitPanel->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.f)); // transparent
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
    PrevHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    PrevHS->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

    CarouselBox = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* CarouselHS = CarouselRow->AddChildToHorizontalBox(CarouselBox);
    CarouselHS->SetVerticalAlignment(VAlign_Center);
    CarouselHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

    BtnNext = WidgetTree->ConstructWidget<UButton>();
    BtnNext->SetStyle(CCMakeButtonStyle(CC_PanelDark, CC_PanelMid));
    BtnNext->AddChild(CCMakeText(WidgetTree, TEXT(" > "), 22, CC_Gold));
    UHorizontalBoxSlot* NextHS = CarouselRow->AddChildToHorizontalBox(BtnNext);
    NextHS->SetVerticalAlignment(VAlign_Center);
    NextHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
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

    // Sous-titre TALENTS
    UTextBlock* TalentsTitle = CCMakeText(WidgetTree, TEXT("TALENTS"), 12, CC_GoldDim);
    UVerticalBoxSlot* TalentsTitleSlot = InfoVBox->AddChildToVerticalBox(TalentsTitle);
    TalentsTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // VBox dynamique — peuplé dans RefreshInfo()
    TalentsVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UVerticalBoxSlot* TalentsVSlot = InfoVBox->AddChildToVerticalBox(TalentsVBox);
    TalentsVSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // Ligne séparatrice
    UBorder* Sep2 = WidgetTree->ConstructWidget<UBorder>();
    Sep2->SetBrushColor(CC_GoldDim);
    UVerticalBoxSlot* Sep2Slot = InfoVBox->AddChildToVerticalBox(Sep2);
    Sep2Slot->SetPadding(FMargin(0.f, 8.f, 0.f, 8.f));
    UBorderSlot* Sep2B = Cast<UBorderSlot>(Sep2->AddChild(WidgetTree->ConstructWidget<UTextBlock>()));
    if (Sep2B) Sep2B->SetPadding(FMargin(0.f, 1.f));

    // Sous-titre ÉQUIPEMENT
    UTextBlock* InvTitle = CCMakeText(WidgetTree, TEXT("ÉQUIPEMENT"), 12, CC_GoldDim);
    UVerticalBoxSlot* InvTitleSlot = InfoVBox->AddChildToVerticalBox(InvTitle);
    InvTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

    // Grille 3x3 — peuplée dans RefreshInfo()
    InventoryGrid = WidgetTree->ConstructWidget<UUniformGridPanel>();
    InventoryGrid->SetSlotPadding(FMargin(2.f));
    UVerticalBoxSlot* GridSlot = InfoVBox->AddChildToVerticalBox(InventoryGrid);
    GridSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // Hint navigation en bas
    UTextBlock* InfoHint = CCMakeText(WidgetTree,
        TEXT("Utilisez < > pour\nnaviguer."),
        11, CC_Grey);
    InfoHint->SetAutoWrapText(true);
    UVerticalBoxSlot* HintSlot = InfoVBox->AddChildToVerticalBox(InfoHint);
    HintSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));

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
    if (!CachedCharacters.IsValidIndex(SelectedIndex)) return;
    const FCharacterInfo& Info = CachedCharacters[SelectedIndex];

    if (CharNameText)
        CharNameText->SetText(Info.DisplayName);

    // Portrait 3D via SceneCapture
    if (PortraitImage)
    {
        if (PreviewActor)
        {
            // Mettre à jour le mesh du preview actor
            PreviewActor->UpdateMesh(Info.PreviewMesh, Info.PreviewAnimClass);

            // Afficher le render target dans le portrait dès qu'il est prêt
            if (PreviewActor->RenderTarget)
            {
                FSlateBrush Brush;
                Brush.SetResourceObject(PreviewActor->RenderTarget);
                Brush.DrawAs    = ESlateBrushDrawType::Image;
                Brush.ImageType = ESlateBrushImageType::FullColor; // active le canal alpha
                Brush.TintColor = FSlateColor(FLinearColor::White);
                PortraitImage->SetBrush(Brush);
            }
        }
        else if (Info.Thumbnail)
            PortraitImage->SetBrushFromTexture(Info.Thumbnail);  // fallback
        else
            PortraitImage->SetColorAndOpacity(FLinearColor(0.2f, 0.17f, 0.13f, 1.f));
    }

    // ── Grille Inventaire ─────────────────────────────────────────────────────
    if (InventoryGrid && WidgetTree)
    {
        InventoryGrid->ClearChildren();
        const TArray<FInventoryItem>& Items = Info.DefaultInventory;
        const int32 MaxSlots = 9;
        for (int32 i = 0; i < MaxSlots; ++i)
        {
            // Fond du slot
            UBorder* SlotBg = WidgetTree->ConstructWidget<UBorder>();
            SlotBg->SetBrushColor(CC_PanelMid);

            USizeBox* SlotSB = WidgetTree->ConstructWidget<USizeBox>();
            SlotSB->SetWidthOverride(36.f);
            SlotSB->SetHeightOverride(36.f);
            SlotSB->AddChild(SlotBg);

            if (Items.IsValidIndex(i) && !Items[i].IsEmpty())
            {
                const FInventoryItem& Item = Items[i];
                UImage* ItemImg = WidgetTree->ConstructWidget<UImage>();
                if (Item.ItemIcon)
                    ItemImg->SetBrushFromTexture(Item.ItemIcon);
                else
                {
                    // Couleur par type si pas d'icône
                    FLinearColor TypeColor = CC_Grey;
                    switch (Item.Type)
                    {
                        case EInventoryItemType::Weapon:    TypeColor = FLinearColor(0.3f, 0.5f, 0.8f, 1.f); break;
                        case EInventoryItemType::Health:    TypeColor = FLinearColor(0.2f, 0.8f, 0.3f, 1.f); break;
                        case EInventoryItemType::Ammo:      TypeColor = FLinearColor(0.8f, 0.7f, 0.2f, 1.f); break;
                        case EInventoryItemType::TimeBonus: TypeColor = FLinearColor(0.8f, 0.3f, 0.8f, 1.f); break;
                        default: break;
                    }
                    ItemImg->SetColorAndOpacity(TypeColor);
                }
                UBorderSlot* ImgBS = Cast<UBorderSlot>(SlotBg->AddChild(ItemImg));
                if (ImgBS)
                {
                    ImgBS->SetHorizontalAlignment(HAlign_Fill);
                    ImgBS->SetVerticalAlignment(VAlign_Fill);
                    ImgBS->SetPadding(FMargin(2.f));
                }
            }

            UUniformGridSlot* GS = InventoryGrid->AddChildToUniformGrid(SlotSB, i / 3, i % 3);
            if (GS)
            {
                GS->SetHorizontalAlignment(HAlign_Center);
                GS->SetVerticalAlignment(VAlign_Center);
            }
        }
    }

    // ── Panneau Talents ───────────────────────────────────────────────────────
    if (!TalentsVBox || !WidgetTree) return;
    TalentsVBox->ClearChildren();

    if (Info.Talents.IsEmpty())
    {
        UTextBlock* NoTalent = CCMakeText(WidgetTree, TEXT("Aucun talent assigné."), 11, CC_Grey);
        NoTalent->SetAutoWrapText(true);
        TalentsVBox->AddChildToVerticalBox(NoTalent);
        return;
    }

    for (const TObjectPtr<UTalentDefinition>& Talent : Info.Talents)
    {
        if (!Talent) continue;

        // Ligne = icône (si dispo) + nom + description
        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
        UVerticalBoxSlot* RowSlot = TalentsVBox->AddChildToVerticalBox(Row);
        RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

        // Icône talent (petit carré)
        USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>();
        IconBox->SetWidthOverride(28.f);
        IconBox->SetHeightOverride(28.f);
        UImage* IconImg = WidgetTree->ConstructWidget<UImage>();
        if (Talent->Icon)
            IconImg->SetBrushFromTexture(Talent->Icon);
        else
            IconImg->SetColorAndOpacity(FLinearColor(CC_Gold.R, CC_Gold.G, CC_Gold.B, 0.5f));
        IconBox->AddChild(IconImg);
        UHorizontalBoxSlot* IconHS = Row->AddChildToHorizontalBox(IconBox);
        IconHS->SetVerticalAlignment(VAlign_Top);
        IconHS->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));

        // Nom + description
        UVerticalBox* TextVBox = WidgetTree->ConstructWidget<UVerticalBox>();
        UHorizontalBoxSlot* TextHS = Row->AddChildToHorizontalBox(TextVBox);
        TextHS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        TextHS->SetVerticalAlignment(VAlign_Top);

        UTextBlock* NameTxt = CCMakeText(WidgetTree, Talent->DisplayName.ToString(), 12, CC_Gold);
        TextVBox->AddChildToVerticalBox(NameTxt);

        if (!Talent->Description.IsEmpty())
        {
            UTextBlock* DescTxt = CCMakeText(WidgetTree, Talent->Description.ToString(), 10, CC_Grey);
            DescTxt->SetAutoWrapText(true);
            UVerticalBoxSlot* DescSlot = TextVBox->AddChildToVerticalBox(DescTxt);
            DescSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
        }

        // Résumé des bonus non-nuls
        FString BonusStr;
        if (Talent->ReloadSpeedBonus     > 0.f) BonusStr += FString::Printf(TEXT("  Rechargement +%.0f%%\n"), Talent->ReloadSpeedBonus * 100.f);
        if (Talent->DamageResistanceBonus> 0.f) BonusStr += FString::Printf(TEXT("  Résistance +%.0f%%\n"),   Talent->DamageResistanceBonus * 100.f);
        if (Talent->AmmoCapacityBonus    > 0.f) BonusStr += FString::Printf(TEXT("  Munitions +%.0f%%\n"),    Talent->AmmoCapacityBonus * 100.f);
        if (Talent->MoveSpeedBonus       > 0.f) BonusStr += FString::Printf(TEXT("  Vitesse +%.0f%%\n"),       Talent->MoveSpeedBonus * 100.f);
        if (Talent->MaxHealthBonus       > 0.f) BonusStr += FString::Printf(TEXT("  Vie +%.0f%%\n"),           Talent->MaxHealthBonus * 100.f);
        if (Talent->StaminaBonus         > 0.f) BonusStr += FString::Printf(TEXT("  Endurance +%.0f%%\n"),     Talent->StaminaBonus * 100.f);
        BonusStr.TrimEndInline();

        if (!BonusStr.IsEmpty())
        {
            UTextBlock* BonusTxt = CCMakeText(WidgetTree, BonusStr, 10, FLinearColor(0.4f, 0.9f, 0.4f, 1.f));
            UVerticalBoxSlot* BonusSlot = TextVBox->AddChildToVerticalBox(BonusTxt);
            BonusSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
        }
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
