// Copyright RevenantOps. All Rights Reserved.
#include "UI/CharacterSelectWidget.h"
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
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UI/CharacterPreviewActor.h"

static FSlateBrush CSW_MakeBrush(FLinearColor Color)
{
    FSlateBrush B; B.TintColor = FSlateColor(Color); return B;
}

static FButtonStyle CSW_MakeButtonStyle(FLinearColor Normal, FLinearColor Hover)
{
    FButtonStyle S;
    S.SetNormal(CSW_MakeBrush(Normal));
    S.SetHovered(CSW_MakeBrush(Hover));
    S.SetPressed(CSW_MakeBrush(Normal * 0.7f));
    return S;
}

static UTextBlock* CSW_MakeText(UWidgetTree* WT, const UUITheme* Theme,
                                const FString& Str, int32 Size,
                                FLinearColor Color, ETextJustify::Type Justify = ETextJustify::Left)
{
    UTextBlock* T = WT->ConstructWidget<UTextBlock>();
    T->SetText(FText::FromString(Str));
    T->SetFont(UUIHelpers::GetFont(Theme, Size));
    T->SetColorAndOpacity(FSlateColor(Color));
    T->SetJustification(Justify);
    return T;
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

TSharedRef<SWidget> UCharacterSelectWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt)
        BuildDefaultUI();
    return Super::RebuildWidget();
}

void UCharacterSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (BtnBack)    { BtnBack->OnClicked.AddDynamic(this,    &UCharacterSelectWidget::HandleBack);    BindButtonSounds(BtnBack); }
    if (BtnPrev)    { BtnPrev->OnClicked.AddDynamic(this,    &UCharacterSelectWidget::HandlePrev);    BindButtonSounds(BtnPrev); }
    if (BtnNext)    { BtnNext->OnClicked.AddDynamic(this,    &UCharacterSelectWidget::HandleNext);    BindButtonSounds(BtnNext); }
    if (BtnConfirm) { BtnConfirm->OnClicked.AddDynamic(this, &UCharacterSelectWidget::HandleConfirm); BindButtonSounds(BtnConfirm); }

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

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

void UCharacterSelectWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    UUITheme* T = GetTheme();
    const FLinearColor C_BgDark    = T ? T->BgDeep      : FLinearColor(0.03f, 0.025f, 0.02f, 1.f);
    const FLinearColor C_Panel     = T ? T->BgPanel      : FLinearColor(0.07f, 0.06f,  0.04f, 1.f);
    const FLinearColor C_PanelMid  = FLinearColor(0.10f, 0.09f, 0.07f, 1.f);
    const FLinearColor C_Gold      = T ? T->GoldTarnish  : FLinearColor(0.85f, 0.70f,  0.30f, 1.f);
    const FLinearColor C_GoldDim   = T ? T->GoldDim      : FLinearColor(0.55f, 0.45f,  0.20f, 1.f);
    const FLinearColor C_White     = T ? T->WhiteText    : FLinearColor(0.95f, 0.93f,  0.88f, 1.f);
    const FLinearColor C_Grey      = T ? T->GreySoft     : FLinearColor(0.45f, 0.42f,  0.38f, 1.f);
    const FLinearColor C_Red       = T ? T->RedBlood     : FLinearColor(0.75f, 0.15f,  0.10f, 1.f);
    const FLinearColor C_CardSel   = FLinearColor(0.15f, 0.12f, 0.07f, 1.f);
    const FLinearColor C_CardUnsel = FLinearColor(0.06f, 0.05f, 0.04f, 1.f);

    // â”€â”€ Root â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(C_BgDark);
    Bg->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Bande rouge en haut
    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(C_Red);
    TopBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* TopBarSlot = Root->AddChildToCanvas(TopBar);
    TopBarSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopBarSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 4.f));
    TopBarSlot->SetAutoSize(true);

    // â”€â”€ Layout principal â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    UVerticalBox* VMain = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VSlot = Root->AddChildToCanvas(VMain);
    VSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    VSlot->SetOffsets(FMargin(60.f, 20.f, 60.f, 20.f));

    // Titre
    UTextBlock* Title = CSW_MakeText(WidgetTree, T, TEXT("SELECTION DU PERSONNAGE"), 30, C_Gold, ETextJustify::Center);
    UVerticalBoxSlot* TitleSlot = VMain->AddChildToVerticalBox(Title);
    TitleSlot->SetHorizontalAlignment(HAlign_Center);
    TitleSlot->SetPadding(FMargin(0.f, 16.f, 0.f, 24.f));

    // â”€â”€ Zone centrale : portrait gauche + carousel + info droite â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    UHorizontalBox* MainRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* MainRowSlot = VMain->AddChildToVerticalBox(MainRow);
    MainRowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    MainRowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));

    // -- Portrait gauche ------
    UBorder* PortraitPanel = WidgetTree->ConstructWidget<UBorder>();
    PortraitPanel->SetBrushColor(UUIHelpers::MakeTransparentBrush().TintColor.GetSpecifiedColor());
    UHorizontalBoxSlot* PortraitHSlot = MainRow->AddChildToHorizontalBox(PortraitPanel);
    PortraitHSlot->SetVerticalAlignment(VAlign_Fill);
    FSlateChildSize PortraitSize(ESlateSizeRule::Fill); PortraitSize.Value = 1.2f;
    PortraitHSlot->SetSize(PortraitSize);
    PortraitHSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

    PortraitImage = WidgetTree->ConstructWidget<UImage>();
    PortraitImage->SetColorAndOpacity(UUIHelpers::WithAlpha(C_Panel, 0.85f));
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
    BtnPrev->SetStyle(CSW_MakeButtonStyle(C_Panel, C_PanelMid));
    BtnPrev->AddChild(CSW_MakeText(WidgetTree, T, TEXT(" < "), 22, C_Gold));
    UHorizontalBoxSlot* PrevHS = CarouselRow->AddChildToHorizontalBox(BtnPrev);
    PrevHS->SetVerticalAlignment(VAlign_Center);
    PrevHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    PrevHS->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

    CarouselBox = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* CarouselHS = CarouselRow->AddChildToHorizontalBox(CarouselBox);
    CarouselHS->SetVerticalAlignment(VAlign_Center);
    CarouselHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

    BtnNext = WidgetTree->ConstructWidget<UButton>();
    BtnNext->SetStyle(CSW_MakeButtonStyle(C_Panel, C_PanelMid));
    BtnNext->AddChild(CSW_MakeText(WidgetTree, T, TEXT(" > "), 22, C_Gold));
    UHorizontalBoxSlot* NextHS = CarouselRow->AddChildToHorizontalBox(BtnNext);
    NextHS->SetVerticalAlignment(VAlign_Center);
    NextHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    NextHS->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));

    // Nom du perso
    CharNameText = CSW_MakeText(WidgetTree, T, TEXT("---"), 24, C_White, ETextJustify::Center);
    UVerticalBoxSlot* NameVSlot = CenterVBox->AddChildToVerticalBox(CharNameText);
    NameVSlot->SetHorizontalAlignment(HAlign_Center);
    NameVSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // -- Info droite ----------
    UBorder* InfoPanel = WidgetTree->ConstructWidget<UBorder>();
    InfoPanel->SetBrushColor(C_Panel);
    UHorizontalBoxSlot* InfoHSlot = MainRow->AddChildToHorizontalBox(InfoPanel);
    FSlateChildSize InfoSize(ESlateSizeRule::Fill); InfoSize.Value = 1.2f;
    InfoHSlot->SetSize(InfoSize);
    InfoHSlot->SetVerticalAlignment(VAlign_Fill);
    InfoHSlot->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));

    UVerticalBox* InfoVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UBorderSlot* InfoBSlot = Cast<UBorderSlot>(InfoPanel->AddChild(InfoVBox));
    if (InfoBSlot) InfoBSlot->SetPadding(FMargin(16.f, 16.f));

    UTextBlock* InfoTitle = CSW_MakeText(WidgetTree, T, TEXT("MERCENAIRE"), 14, C_GoldDim);
    UVerticalBoxSlot* InfoTitleSlot = InfoVBox->AddChildToVerticalBox(InfoTitle);
    InfoTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));

    // Ligne dÃ©corative
    UBorder* Sep = WidgetTree->ConstructWidget<UBorder>();
    Sep->SetBrushColor(C_Red);
    UVerticalBoxSlot* SepSlot = InfoVBox->AddChildToVerticalBox(Sep);
    SepSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
    UBorderSlot* SepBSlot = Cast<UBorderSlot>(Sep->AddChild(WidgetTree->ConstructWidget<UTextBlock>()));
    if (SepBSlot) SepBSlot->SetPadding(FMargin(0.f, 1.f));

    // Sous-titre TALENTS
    UTextBlock* TalentsTitle = CSW_MakeText(WidgetTree, T, TEXT("TALENTS"), 12, C_GoldDim);
    UVerticalBoxSlot* TalentsTitleSlot = InfoVBox->AddChildToVerticalBox(TalentsTitle);
    TalentsTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // VBox dynamique â€” peuplÃ© dans RefreshInfo()
    TalentsVBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UVerticalBoxSlot* TalentsVSlot = InfoVBox->AddChildToVerticalBox(TalentsVBox);
    TalentsVSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // Ligne sÃ©paratrice
    UBorder* Sep2 = WidgetTree->ConstructWidget<UBorder>();
    Sep2->SetBrushColor(C_GoldDim);
    UVerticalBoxSlot* Sep2Slot = InfoVBox->AddChildToVerticalBox(Sep2);
    Sep2Slot->SetPadding(FMargin(0.f, 8.f, 0.f, 8.f));
    UBorderSlot* Sep2B = Cast<UBorderSlot>(Sep2->AddChild(WidgetTree->ConstructWidget<UTextBlock>()));
    if (Sep2B) Sep2B->SetPadding(FMargin(0.f, 1.f));

    // Sous-titre Ã‰QUIPEMENT
    UTextBlock* InvTitle = CSW_MakeText(WidgetTree, T, TEXT("Ã‰QUIPEMENT"), 12, C_GoldDim);
    UVerticalBoxSlot* InvTitleSlot = InfoVBox->AddChildToVerticalBox(InvTitle);
    InvTitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

    // Grille 3x3 â€” peuplÃ©e dans RefreshInfo()
    InventoryGrid = WidgetTree->ConstructWidget<UUniformGridPanel>();
    InventoryGrid->SetSlotPadding(FMargin(2.f));
    UVerticalBoxSlot* GridSlot = InfoVBox->AddChildToVerticalBox(InventoryGrid);
    GridSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // Hint navigation en bas
    UTextBlock* InfoHint = CSW_MakeText(WidgetTree, T,
        TEXT("Utilisez < > pour\nnaviguer."),
        11, C_Grey);
    InfoHint->SetAutoWrapText(true);
    UVerticalBoxSlot* HintSlot = InfoVBox->AddChildToVerticalBox(InfoHint);
    HintSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));

    // â”€â”€ Boutons bas â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    UHorizontalBox* BtnRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* BtnRowSlot = VMain->AddChildToVerticalBox(BtnRow);
    BtnRowSlot->SetHorizontalAlignment(HAlign_Fill);

    BtnBack = WidgetTree->ConstructWidget<UButton>();
    BtnBack->SetStyle(CSW_MakeButtonStyle(C_PanelMid, UUIHelpers::WithAlpha(C_Gold, 0.15f)));
    BtnBack->AddChild(CSW_MakeText(WidgetTree, T, TEXT("< RETOUR"), 16, C_Grey));
    UHorizontalBoxSlot* BackHS = BtnRow->AddChildToHorizontalBox(BtnBack);
    BackHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    BackHS->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));

    UHorizontalBox* Spacer = WidgetTree->ConstructWidget<UHorizontalBox>();
    UHorizontalBoxSlot* SpacerHS = BtnRow->AddChildToHorizontalBox(Spacer);
    SpacerHS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    BtnConfirm = WidgetTree->ConstructWidget<UButton>();
    BtnConfirm->SetStyle(CSW_MakeButtonStyle(UUIHelpers::WithAlpha(C_Red, 0.85f), C_Red));
    BtnConfirm->AddChild(CSW_MakeText(WidgetTree, T, TEXT("SELECTIONNER  >"), 16, C_White));
    UHorizontalBoxSlot* ConfirmHS = BtnRow->AddChildToHorizontalBox(BtnConfirm);
    ConfirmHS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

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

    UUITheme* T = GetTheme();
    const FLinearColor C_White     = T ? T->WhiteText : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
    const FLinearColor C_Grey      = T ? T->GreySoft  : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
    const FLinearColor C_Panel     = T ? T->BgPanel   : FLinearColor(0.07f, 0.06f, 0.04f, 1.f);
    const FLinearColor C_CardSel   = FLinearColor(0.15f, 0.12f, 0.07f, 1.f);
    const FLinearColor C_CardUnsel = FLinearColor(0.06f, 0.05f, 0.04f, 1.f);

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
        Card->SetBrushColor(bSelected ? C_CardSel : C_CardUnsel);

        UVerticalBox* CardVBox = WidgetTree->ConstructWidget<UVerticalBox>();
        UBorderSlot* CardBSlot = Cast<UBorderSlot>(Card->AddChild(CardVBox));
        if (CardBSlot) CardBSlot->SetPadding(FMargin(3.f));

        UImage* Thumb = WidgetTree->ConstructWidget<UImage>();
        if (CachedCharacters[Idx].Thumbnail)
            Thumb->SetBrushFromTexture(CachedCharacters[Idx].Thumbnail);
        else
            Thumb->SetColorAndOpacity(UUIHelpers::WithAlpha(C_Panel, bSelected ? 1.f : 0.5f));

        USizeBox* ThumbSB = WidgetTree->ConstructWidget<USizeBox>();
        ThumbSB->SetWidthOverride(W - 6.f);
        ThumbSB->SetHeightOverride(H - 28.f);
        ThumbSB->AddChild(Thumb);
        UVerticalBoxSlot* ThumbVSlot = CardVBox->AddChildToVerticalBox(ThumbSB);
        ThumbVSlot->SetHorizontalAlignment(HAlign_Center);

        UTextBlock* CardName = CSW_MakeText(WidgetTree, T,
            CachedCharacters[Idx].DisplayName.ToString(),
            bSelected ? 12 : 9,
            bSelected ? C_White : C_Grey,
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
            // Mettre Ã  jour le mesh du preview actor
            PreviewActor->UpdateMesh(Info.PreviewMesh, Info.PreviewAnimClass);

            // Afficher le render target dans le portrait dÃ¨s qu'il est prÃªt
            if (PreviewActor->RenderTarget)
            {
                FSlateBrush Brush;
                Brush.SetResourceObject(PreviewActor->RenderTarget);
                Brush.DrawAs    = ESlateBrushDrawType::Image;
                Brush.ImageType = ESlateBrushImageType::FullColor;
                Brush.TintColor = FSlateColor(FLinearColor::White);
                PortraitImage->SetBrush(Brush);
            }
        }
        else if (Info.Thumbnail)
            PortraitImage->SetBrushFromTexture(Info.Thumbnail);  // fallback
        else
        {
            UUITheme* Th = GetTheme();
            PortraitImage->SetColorAndOpacity(Th ? UUIHelpers::WithAlpha(Th->BgPanel, 0.85f)
                                                 : FLinearColor(0.07f, 0.06f, 0.04f, 0.85f));
        }
    }

    // â”€â”€ Grille Inventaire â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (InventoryGrid && WidgetTree)
    {
        InventoryGrid->ClearChildren();
        UUITheme* T = GetTheme();
        const FLinearColor C_PanelMid = FLinearColor(0.10f, 0.09f, 0.07f, 1.f);
        const FLinearColor C_Grey     = T ? T->GreySoft : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);

        const TArray<FInventoryItem>& Items = Info.DefaultInventory;
        const int32 MaxSlots = 9;
        for (int32 i = 0; i < MaxSlots; ++i)
        {
            // Fond du slot
            UBorder* SlotBg = WidgetTree->ConstructWidget<UBorder>();
            SlotBg->SetBrushColor(C_PanelMid);

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
                    // Couleur par type si pas d'icÃ´ne
                    FLinearColor TypeColor = C_Grey;
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

    // â”€â”€ Panneau Talents â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (!TalentsVBox || !WidgetTree) return;
    TalentsVBox->ClearChildren();

    UUITheme* T = GetTheme();
    const FLinearColor C_Gold    = T ? T->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
    const FLinearColor C_Grey    = T ? T->GreySoft    : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
    const FLinearColor C_Bonus   = FLinearColor(0.4f, 0.9f, 0.4f, 1.f); // vert stat â€” pas dans theme

    if (Info.Talents.IsEmpty())
    {
        UTextBlock* NoTalent = CSW_MakeText(WidgetTree, T, TEXT("Aucun talent assignÃ©."), 11, C_Grey);
        NoTalent->SetAutoWrapText(true);
        TalentsVBox->AddChildToVerticalBox(NoTalent);
        return;
    }

    for (const TObjectPtr<UTalentDefinition>& Talent : Info.Talents)
    {
        if (!Talent) continue;

        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
        UVerticalBoxSlot* RowSlot = TalentsVBox->AddChildToVerticalBox(Row);
        RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

        // IcÃ´ne talent (petit carrÃ©)
        USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>();
        IconBox->SetWidthOverride(28.f);
        IconBox->SetHeightOverride(28.f);
        UImage* IconImg = WidgetTree->ConstructWidget<UImage>();
        if (Talent->Icon)
            IconImg->SetBrushFromTexture(Talent->Icon);
        else
            IconImg->SetColorAndOpacity(UUIHelpers::WithAlpha(C_Gold, 0.5f));
        IconBox->AddChild(IconImg);
        UHorizontalBoxSlot* IconHS = Row->AddChildToHorizontalBox(IconBox);
        IconHS->SetVerticalAlignment(VAlign_Top);
        IconHS->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));

        // Nom + description
        UVerticalBox* TextVBox = WidgetTree->ConstructWidget<UVerticalBox>();
        UHorizontalBoxSlot* TextHS = Row->AddChildToHorizontalBox(TextVBox);
        TextHS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        TextHS->SetVerticalAlignment(VAlign_Top);

        UTextBlock* NameTxt = CSW_MakeText(WidgetTree, T, Talent->DisplayName.ToString(), 12, C_Gold);
        TextVBox->AddChildToVerticalBox(NameTxt);

        if (!Talent->Description.IsEmpty())
        {
            UTextBlock* DescTxt = CSW_MakeText(WidgetTree, T, Talent->Description.ToString(), 10, C_Grey);
            DescTxt->SetAutoWrapText(true);
            UVerticalBoxSlot* DescSlot = TextVBox->AddChildToVerticalBox(DescTxt);
            DescSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
        }

        // RÃ©sumÃ© des bonus non-nuls
        FString BonusStr;
        if (Talent->ReloadSpeedBonus     > 0.f) BonusStr += FString::Printf(TEXT("  Rechargement +%.0f%%\n"), Talent->ReloadSpeedBonus * 100.f);
        if (Talent->DamageResistanceBonus> 0.f) BonusStr += FString::Printf(TEXT("  RÃ©sistance +%.0f%%\n"),   Talent->DamageResistanceBonus * 100.f);
        if (Talent->AmmoCapacityBonus    > 0.f) BonusStr += FString::Printf(TEXT("  Munitions +%.0f%%\n"),    Talent->AmmoCapacityBonus * 100.f);
        if (Talent->MoveSpeedBonus       > 0.f) BonusStr += FString::Printf(TEXT("  Vitesse +%.0f%%\n"),       Talent->MoveSpeedBonus * 100.f);
        if (Talent->MaxHealthBonus       > 0.f) BonusStr += FString::Printf(TEXT("  Vie +%.0f%%\n"),           Talent->MaxHealthBonus * 100.f);
        if (Talent->StaminaBonus         > 0.f) BonusStr += FString::Printf(TEXT("  Endurance +%.0f%%\n"),     Talent->StaminaBonus * 100.f);
        BonusStr.TrimEndInline();

        if (!BonusStr.IsEmpty())
        {
            UTextBlock* BonusTxt = CSW_MakeText(WidgetTree, T, BonusStr, 10, C_Bonus);
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
    PlayClickSound();
    if (CachedCharacters.IsValidIndex(SelectedIndex))
        OnCharacterChosen.Broadcast(CachedCharacters[SelectedIndex]);
}

void UCharacterSelectWidget::HandleBack()
{
    PlayClickSound();
    OnBackClicked.Broadcast();
}
