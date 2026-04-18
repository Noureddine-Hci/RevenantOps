// Copyright RevenantOps. All Rights Reserved.
#include "UI/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/KismetSystemLibrary.h"

// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> UMainMenuWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt)
    {
        BuildDefaultUI();
    }
    return Super::RebuildWidget();
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (BtnPlay)    BtnPlay->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlay);
    if (BtnOptions) BtnOptions->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleOptions);
    if (BtnQuit)    BtnQuit->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuit);
}

// ─────────────────────────────────────────────────────────────────────────────

void UMainMenuWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    // Root canvas
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    // Full-screen background — image si assignée, sinon couleur unie
    UImage* BgImage = WidgetTree->ConstructWidget<UImage>();
    if (BackgroundImage)
    {
        BgImage->SetBrushFromTexture(BackgroundImage, false);
    }
    else
    {
        FSlateBrush SolidBrush;
        SolidBrush.TintColor = FSlateColor(FLinearColor(0.02f, 0.02f, 0.05f, 1.f));
        SolidBrush.DrawAs = ESlateBrushDrawType::Box;
        BgImage->SetBrush(SolidBrush);
    }
    BgImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(BgImage);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Center column
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VSlot = Root->AddChildToCanvas(VBox);
    VSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    VSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    VSlot->SetSize(FVector2D(340.f, 340.f));

    // Game title
    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
    Title->SetText(FText::FromString("REVENANTOPS"));
    FSlateFontInfo TitleFont = Title->GetFont();
    TitleFont.Size = 42;
    Title->SetFont(TitleFont);
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.2f, 0.1f, 1.f)));
    UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(Title);
    TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 48.f));
    TitleSlot->SetHorizontalAlignment(HAlign_Center);

    // Helper — create a styled menu button
    auto MakeBtn = [&](const FString& Label) -> UButton*
    {
        UButton* Btn = WidgetTree->ConstructWidget<UButton>();

        FSlateBrush Normal;
        Normal.TintColor = FSlateColor(FLinearColor(0.08f, 0.08f, 0.18f, 1.f));
        FSlateBrush Hovered;
        Hovered.TintColor = FSlateColor(FLinearColor(0.18f, 0.18f, 0.38f, 1.f));
        FSlateBrush Pressed;
        Pressed.TintColor = FSlateColor(FLinearColor(0.8f, 0.2f, 0.1f, 1.f));

        FButtonStyle Style;
        Style.SetNormal(Normal).SetHovered(Hovered).SetPressed(Pressed);
        Btn->SetStyle(Style);

        UTextBlock* Lbl = WidgetTree->ConstructWidget<UTextBlock>();
        Lbl->SetText(FText::FromString(Label));
        FSlateFontInfo F = Lbl->GetFont();
        F.Size = 20;
        Lbl->SetFont(F);
        Lbl->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        Lbl->SetJustification(ETextJustify::Center);
        Btn->AddChild(Lbl);

        UVerticalBoxSlot* S = VBox->AddChildToVerticalBox(Btn);
        S->SetPadding(FMargin(0.f, 8.f));
        S->SetHorizontalAlignment(HAlign_Fill);
        S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

        return Btn;
    };

    BtnPlay    = MakeBtn("JOUER");
    BtnOptions = MakeBtn("OPTIONS");
    BtnQuit    = MakeBtn("QUITTER");
}

// ─────────────────────────────────────────────────────────────────────────────

void UMainMenuWidget::HandlePlay()    { OnPlayClicked.Broadcast(); }
void UMainMenuWidget::HandleOptions() { OnOptionsClicked.Broadcast(); }
void UMainMenuWidget::HandleQuit()    { OnQuitClicked.Broadcast(); }
