// Copyright RevenantOps. All Rights Reserved.

#include "InventoryWidget.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
#include "Styling/CoreStyle.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"

// ─────────────────────────────────────────────────────────────────────────────
// Color helpers
// ─────────────────────────────────────────────────────────────────────────────

FLinearColor UInventoryWidget::SlotColor(EInventoryItemType Type) {
  switch (Type) {
    case EInventoryItemType::Weapon:    return FLinearColor(0.05f, 0.15f, 0.25f, 1.f);
    case EInventoryItemType::Health:    return FLinearColor(0.25f, 0.05f, 0.05f, 1.f);
    case EInventoryItemType::Ammo:      return FLinearColor(0.05f, 0.22f, 0.05f, 1.f);
    case EInventoryItemType::TimeBonus: return FLinearColor(0.18f, 0.05f, 0.28f, 1.f);
    default:                            return FLinearColor(0.07f, 0.07f, 0.07f, 1.f);
  }
}

FLinearColor UInventoryWidget::BorderColor(EInventoryItemType Type) {
  switch (Type) {
    case EInventoryItemType::Weapon:    return FLinearColor(0.3f, 0.7f, 1.f,  1.f);
    case EInventoryItemType::Health:    return FLinearColor(1.f,  0.3f, 0.3f, 1.f);
    case EInventoryItemType::Ammo:      return FLinearColor(0.3f, 1.f,  0.4f, 1.f);
    case EInventoryItemType::TimeBonus: return FLinearColor(0.8f, 0.4f, 1.f,  1.f);
    default:                            return FLinearColor(0.2f, 0.2f, 0.2f, 1.f);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// RebuildWidget / NativeConstruct
// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> UInventoryWidget::RebuildWidget() {
  // IsDesignTime() = true dans l'editeur → on ne touche pas au WidgetTree
  // IsDesignTime() = false en PIE/runtime → on construit l'UI avant que Slate soit cree
  if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt) {
    BuildDefaultUI();
  }
  return Super::RebuildWidget();
}

void UInventoryWidget::NativeConstruct() {
  Super::NativeConstruct();
  if (CachedItems.Num() == 0) {
    CachedItems.SetNum(SLOT_COUNT);
  }
  UpdateVisuals();
  SetKeyboardFocus();
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildDefaultUI
// ─────────────────────────────────────────────────────────────────────────────

void UInventoryWidget::BuildDefaultUI() {
  if (!WidgetTree) return;
  bUIBuilt = true;

  UUITheme* T = UUIHelpers::GetDefaultTheme();
  const FLinearColor C_Panel   = T ? T->BgPanel     : FLinearColor(0.07f, 0.06f, 0.04f, 1.f);
  const FLinearColor C_Gold    = T ? T->GoldTarnish  : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
  const FLinearColor C_Grey    = T ? T->GreySoft     : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);
  const FLinearColor C_White   = T ? T->WhiteText    : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);

  UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
  WidgetTree->RootWidget = Root;

  // ── Full-screen dim overlay ──────────────────────────────────────────────
  UBorder* Overlay = WidgetTree->ConstructWidget<UBorder>();
  Overlay->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.45f));
  {
    UCanvasPanelSlot* S = Root->AddChildToCanvas(Overlay);
    S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    S->SetOffsets(FMargin(0.f));
    S->SetZOrder(0);
  }

  // ── Main panel ───────────────────────────────────────────────────────────
  UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
  Panel->SetBrushColor(UUIHelpers::WithAlpha(C_Panel, 0.92f));
  {
    UCanvasPanelSlot* S = Root->AddChildToCanvas(Panel);
    S->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    S->SetAlignment(FVector2D(0.5f, 0.5f));
    S->SetSize(FVector2D(360.f, 460.f));
    S->SetZOrder(1);
  }

  // ── Vertical layout inside panel ─────────────────────────────────────────
  UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
  if (UBorderSlot* BS = Cast<UBorderSlot>(Panel->AddChild(VBox))) {
    BS->SetPadding(FMargin(20.f, 16.f));
    BS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
    BS->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
  }

  FSlateFontInfo TitleFont = UUIHelpers::GetFont(T, 22);
  FSlateFontInfo QtyFont   = UUIHelpers::GetMonoFont(T, 9);
  FSlateFontInfo InfoFont  = UUIHelpers::GetFont(T, 14);
  FSlateFontInfo HintFont  = UUIHelpers::GetFont(T, 11);

  // Title
  UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
  Title->SetText(FText::FromString(TEXT("INVENTAIRE")));
  Title->SetFont(TitleFont);
  Title->SetColorAndOpacity(FSlateColor(C_Gold));
  Title->SetJustification(ETextJustify::Center);
  {
    UVerticalBoxSlot* S = VBox->AddChildToVerticalBox(Title);
    S->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
    S->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
  }

  // ── 3x3 grid ─────────────────────────────────────────────────────────────
  const float SlotSize = 84.f;
  const float Gap      = 8.f;

  SlotBorders.SetNum(SLOT_COUNT);
  SlotImages.SetNum(SLOT_COUNT);
  SlotQtyTexts.SetNum(SLOT_COUNT);

  for (int32 Row = 0; Row < COLS; ++Row) {
    UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* HSlot = VBox->AddChildToVerticalBox(HBox);
    HSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
    HSlot->SetPadding(FMargin(0.f, 0.f, 0.f, Row < 2 ? Gap : 0.f));

    for (int32 Col = 0; Col < COLS; ++Col) {
      const int32 Idx = Row * COLS + Col;

      // Outer border (selection highlight)
      UBorder* OuterBorder = WidgetTree->ConstructWidget<UBorder>();
      OuterBorder->SetBrushColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.f));
      UHorizontalBoxSlot* HS = HBox->AddChildToHorizontalBox(OuterBorder);
      HS->SetPadding(FMargin(Col > 0 ? Gap : 0.f, 0.f, 0.f, 0.f));
      HS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
      SlotBorders[Idx] = OuterBorder;

      // Inner content (Canvas for absolute positioning)
      UCanvasPanel* SlotCanvas = WidgetTree->ConstructWidget<UCanvasPanel>();
      if (UBorderSlot* BS = Cast<UBorderSlot>(OuterBorder->AddChild(SlotCanvas))) {
        BS->SetPadding(FMargin(2.f)); // 2px padding = selection border thickness
      }

      // Slot background
      UBorder* SlotBg = WidgetTree->ConstructWidget<UBorder>();
      SlotBg->SetBrushColor(SlotColor(EInventoryItemType::Empty));
      {
        UCanvasPanelSlot* S = SlotCanvas->AddChildToCanvas(SlotBg);
        S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
        S->SetOffsets(FMargin(0.f));
        S->SetSize(FVector2D(SlotSize, SlotSize));
      }

      // Item icon (fills the slot, tinted par la couleur de type si pas d'icone)
      UImage* SlotImg = WidgetTree->ConstructWidget<UImage>();
      SlotImg->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f)); // invisible par defaut
      {
        UCanvasPanelSlot* S = SlotCanvas->AddChildToCanvas(SlotImg);
        S->SetAnchors(FAnchors(0.1f, 0.1f, 0.9f, 0.9f)); // 10% de marge interieure
        S->SetOffsets(FMargin(0.f));
      }
      SlotImages[Idx] = SlotImg;

      // Quantity badge (bottom-right)
      UTextBlock* QtyTxt = WidgetTree->ConstructWidget<UTextBlock>();
      QtyTxt->SetFont(QtyFont);
      QtyTxt->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)));
      QtyTxt->SetJustification(ETextJustify::Right);
      {
        UCanvasPanelSlot* S = SlotCanvas->AddChildToCanvas(QtyTxt);
        S->SetAnchors(FAnchors(1.f, 1.f));
        S->SetAlignment(FVector2D(1.f, 1.f));
        S->SetPosition(FVector2D(-4.f, -3.f));
        S->SetAutoSize(true);
      }
      SlotQtyTexts[Idx] = QtyTxt;
    }
  }

  // ── Separator ────────────────────────────────────────────────────────────
  UBorder* Sep = WidgetTree->ConstructWidget<UBorder>();
  Sep->SetBrushColor(UUIHelpers::WithAlpha(C_Grey, 0.5f));
  {
    UVerticalBoxSlot* S = VBox->AddChildToVerticalBox(Sep);
    S->SetPadding(FMargin(0.f, 14.f, 0.f, 10.f));
  }

  // ── Ligne icone + texte ───────────────────────────────────────────────────
  UHorizontalBox* InfoRow = WidgetTree->ConstructWidget<UHorizontalBox>();
  {
    UVerticalBoxSlot* S = VBox->AddChildToVerticalBox(InfoRow);
    S->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
  }

  // Grande icone a gauche dans le panneau info
  InfoIconImage = WidgetTree->ConstructWidget<UImage>();
  InfoIconImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
  {
    UHorizontalBoxSlot* HS = InfoRow->AddChildToHorizontalBox(InfoIconImage);
    HS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    HS->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
    HS->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
    InfoIconImage->SetDesiredSizeOverride(FVector2D(48.f, 48.f));
  }

  // Colonne nom + description a droite
  UVerticalBox* InfoTextCol = WidgetTree->ConstructWidget<UVerticalBox>();
  {
    UHorizontalBoxSlot* HS = InfoRow->AddChildToHorizontalBox(InfoTextCol);
    HS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    HS->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
  }

  // ── Item name in info panel ───────────────────────────────────────────────
  InfoNameText = WidgetTree->ConstructWidget<UTextBlock>();
  InfoNameText->SetFont(UUIHelpers::GetFont(T, 16));
  InfoNameText->SetColorAndOpacity(FSlateColor(C_Gold));
  InfoNameText->SetJustification(ETextJustify::Left);
  {
    UVerticalBoxSlot* S = InfoTextCol->AddChildToVerticalBox(InfoNameText);
    S->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
  }

  // ── Item description ──────────────────────────────────────────────────────
  InfoDescText = WidgetTree->ConstructWidget<UTextBlock>();
  InfoDescText->SetFont(InfoFont);
  InfoDescText->SetColorAndOpacity(FSlateColor(C_White));
  InfoDescText->SetAutoWrapText(true);
  {
    UVerticalBoxSlot* S = InfoTextCol->AddChildToVerticalBox(InfoDescText);
    S->SetPadding(FMargin(0.f, 0.f, 0.f, 0.f));
  }

  // ── Controls hint ─────────────────────────────────────────────────────────
  InfoHintText = WidgetTree->ConstructWidget<UTextBlock>();
  InfoHintText->SetFont(HintFont);
  InfoHintText->SetColorAndOpacity(FSlateColor(C_Grey));
  InfoHintText->SetText(FText::FromString(TEXT("[E] Utiliser    [Tab/Echap] Fermer")));
  InfoHintText->SetJustification(ETextJustify::Center);
  {
    UVerticalBoxSlot* S = VBox->AddChildToVerticalBox(InfoHintText);
    S->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
    S->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void UInventoryWidget::RefreshSlots(const TArray<FInventoryItem>& Items) {
  CachedItems = Items;
  if (CachedItems.Num() < SLOT_COUNT) {
    CachedItems.SetNum(SLOT_COUNT);
  }
  UpdateVisuals();
}

// ─────────────────────────────────────────────────────────────────────────────
// Input
// ─────────────────────────────────────────────────────────────────────────────

FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& MyGeometry,
                                          const FKeyEvent& InKeyEvent) {
  const FKey Key = InKeyEvent.GetKey();

  // Navigation
  if (Key == EKeys::Z || Key == EKeys::Up)    { MoveSelection(-1,  0); return FReply::Handled(); }
  if (Key == EKeys::S || Key == EKeys::Down)  { MoveSelection(+1,  0); return FReply::Handled(); }
  if (Key == EKeys::Q || Key == EKeys::Left)  { MoveSelection( 0, -1); return FReply::Handled(); }
  if (Key == EKeys::D || Key == EKeys::Right) { MoveSelection( 0, +1); return FReply::Handled(); }

  // Use / equip
  if (Key == EKeys::E || Key == EKeys::Enter) {
    if (SelectedIndex >= 0 && SelectedIndex < CachedItems.Num()) {
      if (!CachedItems[SelectedIndex].IsEmpty()) {
        OnItemUsed.Broadcast(SelectedIndex);
      }
    }
    return FReply::Handled();
  }

  // Close
  if (Key == EKeys::Tab || Key == EKeys::Escape) {
    OnClosed.Broadcast();
    return FReply::Handled();
  }

  return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

// ─────────────────────────────────────────────────────────────────────────────
// Visuals
// ─────────────────────────────────────────────────────────────────────────────

void UInventoryWidget::MoveSelection(int32 DeltaRow, int32 DeltaCol) {
  const int32 Row = (SelectedIndex / COLS + DeltaRow + COLS) % COLS;
  const int32 Col = (SelectedIndex % COLS + DeltaCol + COLS) % COLS;
  SelectedIndex = Row * COLS + Col;
  UpdateVisuals();
  UpdateInfoPanel();
}

void UInventoryWidget::UpdateVisuals() {
  if (SlotBorders.Num() < SLOT_COUNT) return;

  for (int32 i = 0; i < SLOT_COUNT; ++i) {
    const bool bSelected = (i == SelectedIndex);
    const FInventoryItem& Item = (i < CachedItems.Num()) ? CachedItems[i] : FInventoryItem();

    // Bordure de selection
    UUITheme* T2 = UUIHelpers::GetDefaultTheme();
    const FLinearColor C_SelBorder = T2 ? T2->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
    FLinearColor Border;
    if (bSelected) {
      Border = C_SelBorder;
    } else if (!Item.IsEmpty()) {
      Border = BorderColor(Item.Type) * 0.6f;
    } else {
      Border = FLinearColor(0.15f, 0.15f, 0.15f, 1.f);
    }
    if (SlotBorders[i]) SlotBorders[i]->SetBrushColor(Border);

    // Icone du slot
    if (SlotImages[i]) {
      if (!Item.IsEmpty()) {
        if (Item.ItemIcon) {
          // Texture assignee : afficher l'image avec tinte blanc
          SlotImages[i]->SetBrushFromTexture(Item.ItemIcon, true);
          SlotImages[i]->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
        } else {
          // Pas de texture : couleur de type pleine en fallback
          FSlateBrush Brush;
          Brush.TintColor = SlotColor(Item.Type) * 2.f; // un peu plus lumineux
          Brush.DrawAs = ESlateBrushDrawType::Image;
          SlotImages[i]->SetBrush(Brush);
          SlotImages[i]->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.7f));
        }
      } else {
        SlotImages[i]->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f)); // invisible
      }
    }

    // Badge quantite
    if (SlotQtyTexts[i]) {
      if (!Item.IsEmpty() && Item.Quantity > 1) {
        SlotQtyTexts[i]->SetText(
            FText::FromString(FString::Printf(TEXT("x%d"), Item.Quantity)));
      } else {
        SlotQtyTexts[i]->SetText(FText::GetEmpty());
      }
    }
  }

  UpdateInfoPanel();
}

void UInventoryWidget::UpdateInfoPanel() {
  const FInventoryItem& Item =
      (SelectedIndex < CachedItems.Num()) ? CachedItems[SelectedIndex] : FInventoryItem();

  // Grande icone a gauche
  if (InfoIconImage) {
    if (!Item.IsEmpty() && Item.ItemIcon) {
      InfoIconImage->SetBrushFromTexture(Item.ItemIcon, true);
      InfoIconImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
    } else if (!Item.IsEmpty()) {
      // Pas de texture : bloc couleur de type
      FSlateBrush Brush;
      Brush.TintColor = BorderColor(Item.Type);
      Brush.DrawAs = ESlateBrushDrawType::Image;
      InfoIconImage->SetBrush(Brush);
      InfoIconImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.8f));
    } else {
      InfoIconImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
    }
  }

  if (InfoNameText) {
    InfoNameText->SetText(Item.IsEmpty()
                              ? FText::FromString(TEXT("— Vide —"))
                              : Item.DisplayName);
  }
  if (InfoDescText) {
    InfoDescText->SetText(Item.IsEmpty() ? FText::GetEmpty() : Item.Description);
  }
  if (InfoHintText) {
    const FString Hint = Item.IsEmpty()
                             ? TEXT("[Tab/Echap] Fermer")
                             : TEXT("[E] Utiliser    [Tab/Echap] Fermer");
    InfoHintText->SetText(FText::FromString(Hint));
  }
}
