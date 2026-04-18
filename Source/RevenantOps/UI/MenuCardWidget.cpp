// Copyright RevenantOps. All Rights Reserved.
#include "UI/MenuCardWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"

// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> UMenuCardWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt)
    {
        BuildDefaultUI();
    }
    return Super::RebuildWidget();
}

void UMenuCardWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (SelectButton)
    {
        SelectButton->OnClicked.AddDynamic(this, &UMenuCardWidget::HandleClicked);
    }
    // Apply thumbnail if it was set before NativeConstruct
    if (ThumbnailImg && CardThumbnail)
    {
        ThumbnailImg->SetBrushFromTexture(CardThumbnail, true);
    }
    if (NameText)
    {
        NameText->SetText(CardName);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void UMenuCardWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    // SizeBox forces fixed 200x280 card size — avoids FVector2D/FSlateChildSize mismatch
    USizeBox* SizeRoot = WidgetTree->ConstructWidget<USizeBox>();
    SizeRoot->SetWidthOverride(200.f);
    SizeRoot->SetHeightOverride(280.f);
    WidgetTree->RootWidget = SizeRoot;

    // Border for visual background + highlighting
    RootBorder = WidgetTree->ConstructWidget<UBorder>();
    RootBorder->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.14f, 1.f));
    SizeRoot->AddChild(RootBorder);

    // Vertical layout inside the card
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UBorderSlot* BorderSlot = Cast<UBorderSlot>(RootBorder->AddChild(VBox));
    if (BorderSlot)
    {
        BorderSlot->SetPadding(FMargin(8.f));
        BorderSlot->SetHorizontalAlignment(HAlign_Fill);
        BorderSlot->SetVerticalAlignment(VAlign_Fill);
    }

    // Thumbnail
    ThumbnailImg = WidgetTree->ConstructWidget<UImage>();
    FSlateBrush PlaceholderBrush;
    PlaceholderBrush.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.3f, 1.f));
    PlaceholderBrush.DrawAs = ESlateBrushDrawType::Box;
    ThumbnailImg->SetBrush(PlaceholderBrush);
    UVerticalBoxSlot* ImgSlot = VBox->AddChildToVerticalBox(ThumbnailImg);
    ImgSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ImgSlot->SetHorizontalAlignment(HAlign_Fill);

    // Label
    NameText = WidgetTree->ConstructWidget<UTextBlock>();
    NameText->SetText(CardName);
    FSlateFontInfo Font = NameText->GetFont();
    Font.Size = 14;
    NameText->SetFont(Font);
    NameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    NameText->SetJustification(ETextJustify::Center);
    UVerticalBoxSlot* NameSlot = VBox->AddChildToVerticalBox(NameText);
    NameSlot->SetPadding(FMargin(0.f, 6.f, 0.f, 4.f));
    NameSlot->SetHorizontalAlignment(HAlign_Fill);
    NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

    // Select button
    SelectButton = WidgetTree->ConstructWidget<UButton>();
    UTextBlock* BtnLabel = WidgetTree->ConstructWidget<UTextBlock>();
    BtnLabel->SetText(FText::FromString("SELECTIONNER"));
    FSlateFontInfo BtnFont = BtnLabel->GetFont();
    BtnFont.Size = 12;
    BtnLabel->SetFont(BtnFont);
    BtnLabel->SetJustification(ETextJustify::Center);
    SelectButton->AddChild(BtnLabel);

    FSlateBrush BtnNormal;
    BtnNormal.TintColor = FSlateColor(FLinearColor(0.15f, 0.3f, 0.6f, 1.f));
    FButtonStyle BtnStyle;
    BtnStyle.SetNormal(BtnNormal);
    FSlateBrush BtnHover = BtnNormal;
    BtnHover.TintColor = FSlateColor(FLinearColor(0.25f, 0.45f, 0.85f, 1.f));
    BtnStyle.SetHovered(BtnHover);
    FSlateBrush BtnPressed = BtnNormal;
    BtnPressed.TintColor = FSlateColor(FLinearColor(0.8f, 0.2f, 0.1f, 1.f));
    BtnStyle.SetPressed(BtnPressed);
    SelectButton->SetStyle(BtnStyle);

    UVerticalBoxSlot* BtnSlot = VBox->AddChildToVerticalBox(SelectButton);
    BtnSlot->SetHorizontalAlignment(HAlign_Fill);
    BtnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
}

// ─────────────────────────────────────────────────────────────────────────────

void UMenuCardWidget::Setup(int32 InIndex, const FText& InName, UTexture2D* InThumbnail)
{
    CardIndex     = InIndex;
    CardName      = InName;
    CardThumbnail = InThumbnail;

    // If NativeConstruct already ran, update immediately
    if (NameText)     NameText->SetText(CardName);
    if (ThumbnailImg && CardThumbnail)
        ThumbnailImg->SetBrushFromTexture(CardThumbnail, true);
}

void UMenuCardWidget::SetHighlighted(bool bHighlight)
{
    if (!RootBorder) return;
    RootBorder->SetBrushColor(bHighlight
        ? FLinearColor(0.15f, 0.4f, 0.8f, 1.f)
        : FLinearColor(0.08f, 0.08f, 0.14f, 1.f));
}

void UMenuCardWidget::HandleClicked()
{
    OnCardClicked.Broadcast(CardIndex);
}
