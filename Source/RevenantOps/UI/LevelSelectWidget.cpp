// Copyright RevenantOps. All Rights Reserved.
#include "UI/LevelSelectWidget.h"
#include "UI/MenuCardWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"

// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> ULevelSelectWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt)
    {
        BuildDefaultUI();
    }
    return Super::RebuildWidget();
}

void ULevelSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (BtnBack)
    {
        BtnBack->OnClicked.AddDynamic(this, &ULevelSelectWidget::HandleBack);
    }
    // Repopulate if data was passed before AddToViewport (CardContainer was null then)
    if (!CachedLevels.IsEmpty())
    {
        PopulateLevels(CachedLevels);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void ULevelSelectWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    // Full-screen background
    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.06f, 0.95f));
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Center column
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VSlot = Root->AddChildToCanvas(VBox);
    VSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    VSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    VSlot->SetSize(FVector2D(700.f, 520.f));

    // Title
    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
    Title->SetText(FText::FromString("SELECTION DU NIVEAU"));
    FSlateFontInfo TitleFont = Title->GetFont();
    TitleFont.Size = 28;
    Title->SetFont(TitleFont);
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.9f, 0.6f, 1.f)));
    UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(Title);
    TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 24.f));
    TitleSlot->SetHorizontalAlignment(HAlign_Center);

    // Scrollable card area
    CardContainer = WidgetTree->ConstructWidget<UScrollBox>();
    CardContainer->SetOrientation(EOrientation::Orient_Horizontal);
    UVerticalBoxSlot* ScrollSlot = VBox->AddChildToVerticalBox(CardContainer);
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ScrollSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));

    // Back button
    BtnBack = WidgetTree->ConstructWidget<UButton>();
    UTextBlock* BackLbl = WidgetTree->ConstructWidget<UTextBlock>();
    BackLbl->SetText(FText::FromString("< Retour"));
    FSlateFontInfo BackFont = BackLbl->GetFont();
    BackFont.Size = 16;
    BackLbl->SetFont(BackFont);
    BackLbl->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    BtnBack->AddChild(BackLbl);

    FSlateBrush BackNormal;
    BackNormal.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.f));
    FButtonStyle BackStyle;
    BackStyle.SetNormal(BackNormal);
    FSlateBrush BackHover = BackNormal;
    BackHover.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.4f, 1.f));
    BackStyle.SetHovered(BackHover);
    BackStyle.SetPressed(BackNormal);
    BtnBack->SetStyle(BackStyle);

    UVerticalBoxSlot* BackSlot = VBox->AddChildToVerticalBox(BtnBack);
    BackSlot->SetHorizontalAlignment(HAlign_Left);
    BackSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
}

// ─────────────────────────────────────────────────────────────────────────────

void ULevelSelectWidget::PopulateLevels(const TArray<FLevelInfo>& Levels)
{
    CachedLevels = Levels;
    Cards.Empty();

    if (!CardContainer) return;
    CardContainer->ClearChildren();

    for (int32 i = 0; i < Levels.Num(); ++i)
    {
        UMenuCardWidget* Card = CreateWidget<UMenuCardWidget>(
            GetOwningPlayer(), UMenuCardWidget::StaticClass());
        if (!Card) continue;

        Card->Setup(i, Levels[i].DisplayName, Levels[i].Thumbnail);
        Card->OnCardClicked.AddUObject(this, &ULevelSelectWidget::HandleCardClicked);

        UScrollBoxSlot* SSlot = Cast<UScrollBoxSlot>(CardContainer->AddChild(Card));
        if (SSlot)
        {
            SSlot->SetPadding(FMargin(12.f, 0.f));
            // Size enforced by MenuCardWidget's internal SizeBox (200x280)
        }

        Cards.Add(Card);
    }
}

void ULevelSelectWidget::HandleCardClicked(int32 Index)
{
    if (CachedLevels.IsValidIndex(Index))
    {
        // Highlight selected card
        for (int32 i = 0; i < Cards.Num(); ++i)
        {
            if (Cards[i]) Cards[i]->SetHighlighted(i == Index);
        }
        OnLevelChosen.Broadcast(CachedLevels[Index]);
    }
}

void ULevelSelectWidget::HandleBack()
{
    OnBackClicked.Broadcast();
}
