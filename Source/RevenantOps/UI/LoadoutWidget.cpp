// Copyright RevenantOps. All Rights Reserved.

#include "LoadoutWidget.h"
#include "Styling/CoreStyle.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "WeaponBase.h"

TSharedRef<SWidget> ULoadoutWidget::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget && !bDefaultUIBuilt) {
    BuildDefaultUI();
  }
  return Super::RebuildWidget();
}

void ULoadoutWidget::NativeConstruct() {
  Super::NativeConstruct();

  if (AvailableWeapons.Num() >= 2 && PrimaryWeaponIndex < 0) {
    PrimaryWeaponIndex   = 0;
    SecondaryWeaponIndex = 1;
  }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

FString ULoadoutWidget::GetWeaponDisplayName(int32 Index) const {
  if (Index < 0 || Index >= AvailableWeapons.Num()) return TEXT("---");
  const FWeaponLoadoutInfo &Info = AvailableWeapons[Index];
  return Info.WeaponName.IsEmpty()
             ? FString::Printf(TEXT("Arme %d"), Index + 1)
             : Info.WeaponName.ToString();
}

FString ULoadoutWidget::GetWeaponStatsLine(int32 Index) const {
  if (Index < 0 || Index >= AvailableWeapons.Num()) return TEXT("---");
  const FWeaponLoadoutInfo &Info = AvailableWeapons[Index];
  return FString::Printf(TEXT("DMG:%.0f  ROF:%.0f  MAG:%d  RLD:%.1fs"),
                         Info.Damage, Info.FireRate, Info.MagazineSize,
                         Info.ReloadTime);
}

void ULoadoutWidget::CycleSlot(int32 &SlotIndex, int32 OtherIndex, int32 Dir) {
  const int32 N = AvailableWeapons.Num();
  if (N <= 1) return;
  int32 Next  = (SlotIndex + Dir + N) % N;
  int32 Tries = N;
  while (Next == OtherIndex && --Tries > 0) {
    Next = (Next + Dir + N) % N;
  }
  SlotIndex = Next;
  RefreshWeaponButtons();
  BP_OnSelectionChanged((&SlotIndex == &PrimaryWeaponIndex) ? 0 : 1, SlotIndex);
}

// ---------------------------------------------------------------------------
// Button arrow callbacks
// ---------------------------------------------------------------------------

void ULoadoutWidget::OnPrimaryLeft()    { CycleSlot(PrimaryWeaponIndex,   SecondaryWeaponIndex, -1); }
void ULoadoutWidget::OnPrimaryRight()   { CycleSlot(PrimaryWeaponIndex,   SecondaryWeaponIndex, +1); }
void ULoadoutWidget::OnSecondaryLeft()  { CycleSlot(SecondaryWeaponIndex, PrimaryWeaponIndex,   -1); }
void ULoadoutWidget::OnSecondaryRight() { CycleSlot(SecondaryWeaponIndex, PrimaryWeaponIndex,   +1); }

// ---------------------------------------------------------------------------
// UI refresh
// ---------------------------------------------------------------------------

void ULoadoutWidget::RefreshWeaponButtons() {
  if (PrimaryNameText)
    PrimaryNameText->SetText(FText::FromString(GetWeaponDisplayName(PrimaryWeaponIndex)));
  if (PrimaryStatsText)
    PrimaryStatsText->SetText(FText::FromString(GetWeaponStatsLine(PrimaryWeaponIndex)));
  if (SecondaryNameText)
    SecondaryNameText->SetText(FText::FromString(GetWeaponDisplayName(SecondaryWeaponIndex)));
  if (SecondaryStatsText)
    SecondaryStatsText->SetText(FText::FromString(GetWeaponStatsLine(SecondaryWeaponIndex)));
}

// ---------------------------------------------------------------------------
// Build UI
// ---------------------------------------------------------------------------

static UButton* MakeArrowButton(UWidgetTree* WT, const FString& Label,
                                 FSlateFontInfo Font) {
  UButton*    Btn  = WT->ConstructWidget<UButton>();
  UTextBlock* Text = WT->ConstructWidget<UTextBlock>();
  Text->SetText(FText::FromString(Label));
  Text->SetFont(Font);
  Text->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.8f, 0.2f)));
  Btn->AddChild(Text);

  FButtonStyle Style = Btn->GetStyle();
  FSlateColor Transparent(FLinearColor(0, 0, 0, 0));
  Style.Normal.TintColor  = Transparent;
  Style.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 0.8f, 0.2f, 0.2f));
  Style.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 0.8f, 0.2f, 0.4f));
  Btn->SetStyle(Style);
  return Btn;
}

/** Build a selector row:  [←]  WeaponName  [→]  */
static UHorizontalBox* MakeSlotRow(UWidgetTree *WT, FSlateFontInfo ArrowFont,
                                    FSlateFontInfo NameFont,
                                    UTextBlock *&OutNameText,
                                    UButton *&OutLeft, UButton *&OutRight) {
  UHorizontalBox *HBox = WT->ConstructWidget<UHorizontalBox>();

  OutLeft = MakeArrowButton(WT, TEXT("  <  "), ArrowFont);
  UHorizontalBoxSlot *LS = HBox->AddChildToHorizontalBox(OutLeft);
  LS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  LS->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

  OutNameText = WT->ConstructWidget<UTextBlock>();
  OutNameText->SetFont(NameFont);
  OutNameText->SetJustification(ETextJustify::Center);
  OutNameText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f)));
  UHorizontalBoxSlot *NS = HBox->AddChildToHorizontalBox(OutNameText);
  NS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  NS->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
  NS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  NS->SetPadding(FMargin(12, 0));

  OutRight = MakeArrowButton(WT, TEXT("  >  "), ArrowFont);
  UHorizontalBoxSlot *RS = HBox->AddChildToHorizontalBox(OutRight);
  RS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  RS->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

  return HBox;
}

void ULoadoutWidget::BuildDefaultUI() {
  if (!WidgetTree) return;
  bDefaultUIBuilt = true;

  UCanvasPanel *Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
  WidgetTree->RootWidget = Canvas;

  // Background
  UImage *Bg = WidgetTree->ConstructWidget<UImage>();
  Bg->SetColorAndOpacity(FLinearColor(0.02f, 0.02f, 0.05f, 0.95f));
  UCanvasPanelSlot *BgSlot = Canvas->AddChildToCanvas(Bg);
  BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
  BgSlot->SetOffsets(FMargin(0.f));
  BgSlot->SetZOrder(0);

  // Centered vertical container
  UVerticalBox *VBox    = WidgetTree->ConstructWidget<UVerticalBox>();
  UCanvasPanelSlot *VS  = Canvas->AddChildToCanvas(VBox);
  VS->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
  VS->SetAlignment(FVector2D(0.5f, 0.5f));
  VS->SetAutoSize(true);
  VS->SetZOrder(1);

  FSlateFontInfo TitleFont  = FCoreStyle::GetDefaultFontStyle("Bold",    32);
  FSlateFontInfo LabelFont  = FCoreStyle::GetDefaultFontStyle("Bold",    16);
  FSlateFontInfo ArrowFont  = FCoreStyle::GetDefaultFontStyle("Bold",    22);
  FSlateFontInfo WeaponFont = FCoreStyle::GetDefaultFontStyle("Regular", 20);
  FSlateFontInfo StatsFont  = FCoreStyle::GetDefaultFontStyle("Regular", 13);
  FSlateFontInfo BtnFont    = FCoreStyle::GetDefaultFontStyle("Regular", 18);

  FLinearColor YellowColor(1.f, 0.8f, 0.2f);
  FLinearColor CyanColor  (0.4f, 0.9f, 1.f);

  auto AddLabel = [&](UVerticalBox *Box, const FString &Str,
                      FSlateFontInfo Font, FLinearColor Color,
                      FMargin Pad) {
    UTextBlock *T = WidgetTree->ConstructWidget<UTextBlock>();
    T->SetText(FText::FromString(Str));
    T->SetFont(Font);
    T->SetColorAndOpacity(FSlateColor(Color));
    T->SetJustification(ETextJustify::Center);
    UVerticalBoxSlot *Slot = Box->AddChildToVerticalBox(T);
    Slot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
    Slot->SetPadding(Pad);
  };

  // Title
  AddLabel(VBox, TEXT("CHOISISSEZ VOS ARMES"), TitleFont, YellowColor,
           FMargin(0, 0, 0, 30));

  // ─── PRIMAIRE ───
  AddLabel(VBox, TEXT("PRIMAIRE"), LabelFont, CyanColor, FMargin(0, 0, 0, 6));

  UButton *BtnPL = nullptr, *BtnPR = nullptr;
  UHorizontalBox *PRow = MakeSlotRow(WidgetTree, ArrowFont, WeaponFont,
                                      PrimaryNameText, BtnPL, BtnPR);
  BtnPL->OnClicked.AddDynamic(this, &ULoadoutWidget::OnPrimaryLeft);
  BtnPR->OnClicked.AddDynamic(this, &ULoadoutWidget::OnPrimaryRight);
  UVerticalBoxSlot *PRSlot = VBox->AddChildToVerticalBox(PRow);
  PRSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  PRSlot->SetPadding(FMargin(0, 0, 0, 4));

  PrimaryStatsText = WidgetTree->ConstructWidget<UTextBlock>();
  PrimaryStatsText->SetFont(StatsFont);
  PrimaryStatsText->SetJustification(ETextJustify::Center);
  PrimaryStatsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)));
  UVerticalBoxSlot *PSSlot = VBox->AddChildToVerticalBox(PrimaryStatsText);
  PSSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  PSSlot->SetPadding(FMargin(0, 0, 0, 24));

  // ─── SECONDAIRE ───
  AddLabel(VBox, TEXT("SECONDAIRE"), LabelFont, CyanColor, FMargin(0, 0, 0, 6));

  UButton *BtnSL = nullptr, *BtnSR = nullptr;
  UHorizontalBox *SRow = MakeSlotRow(WidgetTree, ArrowFont, WeaponFont,
                                      SecondaryNameText, BtnSL, BtnSR);
  BtnSL->OnClicked.AddDynamic(this, &ULoadoutWidget::OnSecondaryLeft);
  BtnSR->OnClicked.AddDynamic(this, &ULoadoutWidget::OnSecondaryRight);
  UVerticalBoxSlot *SRSlot = VBox->AddChildToVerticalBox(SRow);
  SRSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  SRSlot->SetPadding(FMargin(0, 0, 0, 4));

  SecondaryStatsText = WidgetTree->ConstructWidget<UTextBlock>();
  SecondaryStatsText->SetFont(StatsFont);
  SecondaryStatsText->SetJustification(ETextJustify::Center);
  SecondaryStatsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)));
  UVerticalBoxSlot *SSSlot = VBox->AddChildToVerticalBox(SecondaryStatsText);
  SSSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  SSSlot->SetPadding(FMargin(0, 0, 0, 32));

  // ─── CONFIRMER ───
  ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),
                                                       TEXT("ConfirmButton"));
  UTextBlock *ConfirmText = WidgetTree->ConstructWidget<UTextBlock>();
  ConfirmText->SetText(FText::FromString(TEXT("CONFIRMER")));
  ConfirmText->SetFont(BtnFont);
  ConfirmText->SetJustification(ETextJustify::Center);
  ConfirmButton->AddChild(ConfirmText);
  ConfirmButton->OnClicked.AddDynamic(this, &ULoadoutWidget::ConfirmLoadout);
  UVerticalBoxSlot *ConfSlot = VBox->AddChildToVerticalBox(ConfirmButton);
  ConfSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);

  // Auto-select first two weapons
  if (AvailableWeapons.Num() >= 2) {
    PrimaryWeaponIndex   = 0;
    SecondaryWeaponIndex = 1;
  }
  RefreshWeaponButtons();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void ULoadoutWidget::SelectWeapon(int32 WeaponIndex, int32 SlotIndex) {
  if (WeaponIndex < 0 || WeaponIndex >= AvailableWeapons.Num()) return;

  if (SlotIndex == 0) {
    if (WeaponIndex == SecondaryWeaponIndex) SecondaryWeaponIndex = -1;
    PrimaryWeaponIndex = WeaponIndex;
  } else if (SlotIndex == 1) {
    if (WeaponIndex == PrimaryWeaponIndex) PrimaryWeaponIndex = -1;
    SecondaryWeaponIndex = WeaponIndex;
  }

  RefreshWeaponButtons();
  BP_OnSelectionChanged(SlotIndex, WeaponIndex);
}

void ULoadoutWidget::ConfirmLoadout() {
  TSubclassOf<AWeaponBase> Primary   = nullptr;
  TSubclassOf<AWeaponBase> Secondary = nullptr;

  if (CanConfirm()) {
    Primary   = AvailableWeapons[PrimaryWeaponIndex].WeaponClass;
    Secondary = AvailableWeapons[SecondaryWeaponIndex].WeaponClass;
  } else if (AvailableWeapons.Num() >= 2) {
    Primary   = AvailableWeapons[0].WeaponClass;
    Secondary = AvailableWeapons[1].WeaponClass;
  } else if (AvailableWeapons.Num() == 1) {
    Primary   = AvailableWeapons[0].WeaponClass;
    Secondary = AvailableWeapons[0].WeaponClass;
  }

  OnLoadoutConfirmed.Broadcast(Primary, Secondary);
  BP_OnLoadoutConfirmed();
}

bool ULoadoutWidget::CanConfirm() const {
  return PrimaryWeaponIndex >= 0 && SecondaryWeaponIndex >= 0 &&
         PrimaryWeaponIndex != SecondaryWeaponIndex &&
         PrimaryWeaponIndex < AvailableWeapons.Num() &&
         SecondaryWeaponIndex < AvailableWeapons.Num();
}

FWeaponLoadoutInfo ULoadoutWidget::GetWeaponInfo(int32 Index) const {
  if (Index >= 0 && Index < AvailableWeapons.Num()) {
    return AvailableWeapons[Index];
  }
  return FWeaponLoadoutInfo();
}

void ULoadoutWidget::PopulateFromClasses(
    const TArray<TSubclassOf<AWeaponBase>> &WeaponClasses) {
  AvailableWeapons.Empty();

  for (const TSubclassOf<AWeaponBase> &WeapClass : WeaponClasses) {
    if (!WeapClass) continue;
    const AWeaponBase *CDO = WeapClass->GetDefaultObject<AWeaponBase>();
    if (!CDO) continue;

    FWeaponLoadoutInfo Info;
    Info.WeaponClass  = WeapClass;
    Info.WeaponName   = CDO->GetWeaponName();
    Info.Damage       = CDO->GetBaseDamage();
    Info.FireRate     = CDO->GetFireRate();
    Info.MagazineSize = CDO->GetMagazineSize();
    Info.ReloadTime   = CDO->GetReloadTime();
    Info.WeaponIcon   = nullptr;
    AvailableWeapons.Add(Info);
  }

  if (AvailableWeapons.Num() >= 2 && PrimaryWeaponIndex < 0) {
    PrimaryWeaponIndex   = 0;
    SecondaryWeaponIndex = 1;
  }

  RefreshWeaponButtons();
}
