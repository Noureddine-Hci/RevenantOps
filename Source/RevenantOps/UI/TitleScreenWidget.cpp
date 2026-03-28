// Copyright RevenantOps. All Rights Reserved.

#include "TitleScreenWidget.h"
#include "Styling/CoreStyle.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
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
}

void UTitleScreenWidget::BuildDefaultUI() {
  if (!WidgetTree) return;

  // Root canvas panel
  if (!WidgetTree->RootWidget) {
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
  }

  UCanvasPanel* Canvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
  if (!Canvas) return;

  // Full-screen dark background
  UImage* Background = WidgetTree->ConstructWidget<UImage>();
  Background->SetColorAndOpacity(FLinearColor(0.02f, 0.02f, 0.05f, 0.95f));
  UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(Background);
  BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
  BgSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 0.f));
  BgSlot->SetZOrder(0);

  // Vertical box — centered on screen
  UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
  UCanvasPanelSlot* VBoxSlot = Canvas->AddChildToCanvas(VBox);
  VBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
  VBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
  VBoxSlot->SetAutoSize(true);

  // Title text — "REVENANTOPS"
  UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
  Title->SetText(FText::FromString(TEXT("REVENANTOPS")));
  Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 48));
  Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.2f, 0.2f)));
  Title->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(Title);
  TitleSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  TitleSlot->SetPadding(FMargin(0, 0, 0, 60));

  // Subtitle — "Mode Mercenaires"
  UTextBlock* Sub = WidgetTree->ConstructWidget<UTextBlock>();
  Sub->SetText(FText::FromString(TEXT("Mode Mercenaires")));
  Sub->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 20));
  Sub->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* SubSlot = VBox->AddChildToVerticalBox(Sub);
  SubSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  SubSlot->SetPadding(FMargin(0, 0, 0, 40));

  FSlateFontInfo BtnFont = FCoreStyle::GetDefaultFontStyle("Regular", 22);

  // Play Button
  PlayButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PlayButton"));
  UTextBlock* PlayText = WidgetTree->ConstructWidget<UTextBlock>();
  PlayText->SetText(FText::FromString(TEXT("JOUER")));
  PlayText->SetFont(BtnFont);
  PlayText->SetJustification(ETextJustify::Center);
  PlayButton->AddChild(PlayText);
  UVerticalBoxSlot* PlaySlot = VBox->AddChildToVerticalBox(PlayButton);
  PlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  PlaySlot->SetPadding(FMargin(0, 0, 0, 15));

  // Quit Button
  QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("QuitButton"));
  UTextBlock* QuitText = WidgetTree->ConstructWidget<UTextBlock>();
  QuitText->SetText(FText::FromString(TEXT("QUITTER")));
  QuitText->SetFont(BtnFont);
  QuitText->SetJustification(ETextJustify::Center);
  QuitButton->AddChild(QuitText);
  UVerticalBoxSlot* QuitSlot = VBox->AddChildToVerticalBox(QuitButton);
  QuitSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
}

void UTitleScreenWidget::OnPlayClicked() {
  BP_OnPlayPressed();

  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->ShowLoadoutScreen();
  }
}

void UTitleScreenWidget::OnQuitClicked() {
  UKismetSystemLibrary::QuitGame(
      GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
