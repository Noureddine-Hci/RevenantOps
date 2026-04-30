// Copyright RevenantOps. All Rights Reserved.

#include "LoadoutWidget.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
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

static UButton* LW_MakeArrowButton(UWidgetTree* WT, const FString& Label,
                                    FSlateFontInfo Font,
                                    const FLinearColor& AccentColor)
{
    UButton*    Btn  = WT->ConstructWidget<UButton>();
    UTextBlock* Text = WT->ConstructWidget<UTextBlock>();
    Text->SetText(FText::FromString(Label));
    Text->SetFont(Font);
    Text->SetColorAndOpacity(FSlateColor(AccentColor));
    Btn->AddChild(Text);

    FButtonStyle Style = Btn->GetStyle();
    Style.Normal.TintColor  = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
    Style.Hovered.TintColor = FSlateColor(UUIHelpers::WithAlpha(AccentColor, 0.2f));
    Style.Pressed.TintColor = FSlateColor(UUIHelpers::WithAlpha(AccentColor, 0.4f));
    Btn->SetStyle(Style);
    return Btn;
}

/** Build a selector row:  [←]  WeaponName  [→]  */
static UHorizontalBox* LW_MakeSlotRow(UWidgetTree* WT,
                                       FSlateFontInfo ArrowFont, FSlateFontInfo NameFont,
                                       const FLinearColor& AccentColor,
                                       const FLinearColor& TextColor,
                                       UTextBlock*& OutNameText,
                                       UButton*& OutLeft, UButton*& OutRight)
{
    UHorizontalBox* HBox = WT->ConstructWidget<UHorizontalBox>();

    OutLeft = LW_MakeArrowButton(WT, TEXT("  <  "), ArrowFont, AccentColor);
    UHorizontalBoxSlot* LS = HBox->AddChildToHorizontalBox(OutLeft);
    LS->SetHorizontalAlignment(HAlign_Center);
    LS->SetVerticalAlignment(VAlign_Center);

    OutNameText = WT->ConstructWidget<UTextBlock>();
    OutNameText->SetFont(NameFont);
    OutNameText->SetJustification(ETextJustify::Center);
    OutNameText->SetColorAndOpacity(FSlateColor(TextColor));
    UHorizontalBoxSlot* NS = HBox->AddChildToHorizontalBox(OutNameText);
    NS->SetHorizontalAlignment(HAlign_Center);
    NS->SetVerticalAlignment(VAlign_Center);
    NS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    NS->SetPadding(FMargin(12.f, 0.f));

    OutRight = LW_MakeArrowButton(WT, TEXT("  >  "), ArrowFont, AccentColor);
    HBox->AddChildToHorizontalBox(OutRight)->SetVerticalAlignment(VAlign_Center);

    return HBox;
}

void ULoadoutWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bDefaultUIBuilt = true;

    // ── Theme ────────────────────────────────────────────────────────────────
    UUITheme* T = GetTheme();
    const FLinearColor C_Bg      = T ? T->BgDeep      : FLinearColor(0.03f, 0.025f, 0.02f, 1.f);
    const FLinearColor C_Gold    = T ? T->GoldTarnish  : FLinearColor(0.85f, 0.70f,  0.30f, 1.f);
    const FLinearColor C_GoldDim = T ? T->GoldDim      : FLinearColor(0.55f, 0.45f,  0.20f, 1.f);
    const FLinearColor C_White   = T ? T->WhiteText    : FLinearColor(0.95f, 0.93f,  0.88f, 1.f);
    const FLinearColor C_Grey    = T ? T->GreySoft     : FLinearColor(0.45f, 0.42f,  0.38f, 1.f);
    const FLinearColor C_Red     = T ? T->RedBlood     : FLinearColor(0.75f, 0.15f,  0.10f, 1.f);
    const FLinearColor C_Panel   = T ? T->BgPanel      : FLinearColor(0.07f, 0.06f,  0.04f, 1.f);

    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;

    // Fond plein écran
    UImage* Bg = WidgetTree->ConstructWidget<UImage>();
    Bg->SetColorAndOpacity(C_Bg);
    UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));
    BgSlot->SetZOrder(0);

    // Bande rouge haut
    UImage* TopBar = WidgetTree->ConstructWidget<UImage>();
    TopBar->SetColorAndOpacity(C_Red);
    UCanvasPanelSlot* TopSlot = Canvas->AddChildToCanvas(TopBar);
    TopSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 5.f));
    TopSlot->SetAutoSize(true);
    TopSlot->SetZOrder(1);

    // Conteneur central
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VS = Canvas->AddChildToCanvas(VBox);
    VS->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    VS->SetAlignment(FVector2D(0.5f, 0.5f));
    VS->SetAutoSize(true);
    VS->SetZOrder(2);

    FSlateFontInfo TitleFont  = UUIHelpers::GetFont(T, 28);
    FSlateFontInfo LabelFont  = UUIHelpers::GetFont(T, 14);
    FSlateFontInfo ArrowFont  = UUIHelpers::GetFont(T, 20);
    FSlateFontInfo WeaponFont = UUIHelpers::GetFont(T, 18);
    FSlateFontInfo StatsFont  = UUIHelpers::GetFont(T, 12);
    FSlateFontInfo BtnFont    = UUIHelpers::GetFont(T, 16);

    auto AddLabel = [&](const FString& Str, FSlateFontInfo Font,
                        const FLinearColor& Color, FMargin Pad)
    {
        UTextBlock* Tb = WidgetTree->ConstructWidget<UTextBlock>();
        Tb->SetText(FText::FromString(Str));
        Tb->SetFont(Font);
        Tb->SetColorAndOpacity(FSlateColor(Color));
        Tb->SetJustification(ETextJustify::Center);
        UVerticalBoxSlot* S = VBox->AddChildToVerticalBox(Tb);
        S->SetHorizontalAlignment(HAlign_Center);
        S->SetPadding(Pad);
    };

    // Titre "ARMURERIE"
    AddLabel(TEXT("ARMURERIE — CHOIX DES ARMES"), TitleFont, C_Gold, FMargin(0.f, 0.f, 0.f, 28.f));

    // ─── PRIMAIRE ───
    AddLabel(TEXT("PRIMAIRE"), LabelFont, C_GoldDim, FMargin(0.f, 0.f, 0.f, 6.f));

    UButton *BtnPL = nullptr, *BtnPR = nullptr;
    UHorizontalBox* PRow = LW_MakeSlotRow(WidgetTree, ArrowFont, WeaponFont,
                                           C_Gold, C_White,
                                           PrimaryNameText, BtnPL, BtnPR);
    BtnPL->OnClicked.AddDynamic(this, &ULoadoutWidget::OnPrimaryLeft);
    BtnPR->OnClicked.AddDynamic(this, &ULoadoutWidget::OnPrimaryRight);
    BindButtonSounds(BtnPL); BindButtonSounds(BtnPR);
    VBox->AddChildToVerticalBox(PRow)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

    PrimaryStatsText = WidgetTree->ConstructWidget<UTextBlock>();
    PrimaryStatsText->SetFont(StatsFont);
    PrimaryStatsText->SetJustification(ETextJustify::Center);
    PrimaryStatsText->SetColorAndOpacity(FSlateColor(C_Grey));
    VBox->AddChildToVerticalBox(PrimaryStatsText)->SetPadding(FMargin(0.f, 0.f, 0.f, 22.f));

    // ─── SECONDAIRE ───
    AddLabel(TEXT("SECONDAIRE"), LabelFont, C_GoldDim, FMargin(0.f, 0.f, 0.f, 6.f));

    UButton *BtnSL = nullptr, *BtnSR = nullptr;
    UHorizontalBox* SRow = LW_MakeSlotRow(WidgetTree, ArrowFont, WeaponFont,
                                           C_Gold, C_White,
                                           SecondaryNameText, BtnSL, BtnSR);
    BtnSL->OnClicked.AddDynamic(this, &ULoadoutWidget::OnSecondaryLeft);
    BtnSR->OnClicked.AddDynamic(this, &ULoadoutWidget::OnSecondaryRight);
    BindButtonSounds(BtnSL); BindButtonSounds(BtnSR);
    VBox->AddChildToVerticalBox(SRow)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

    SecondaryStatsText = WidgetTree->ConstructWidget<UTextBlock>();
    SecondaryStatsText->SetFont(StatsFont);
    SecondaryStatsText->SetJustification(ETextJustify::Center);
    SecondaryStatsText->SetColorAndOpacity(FSlateColor(C_Grey));
    VBox->AddChildToVerticalBox(SecondaryStatsText)->SetPadding(FMargin(0.f, 0.f, 0.f, 30.f));

    // ─── CONFIRMER ───
    ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ConfirmButton"));
    {
        FButtonStyle BtnStyle;
        BtnStyle.SetNormal (UUIHelpers::MakeSolidBrush(UUIHelpers::WithAlpha(C_Red, 0.85f)));
        BtnStyle.SetHovered(UUIHelpers::MakeSolidBrush(C_Red));
        BtnStyle.SetPressed(UUIHelpers::MakeSolidBrush(UUIHelpers::WithAlpha(C_Red, 0.6f)));
        BtnStyle.NormalPadding  = FMargin(40.f, 12.f);
        BtnStyle.PressedPadding = FMargin(40.f, 13.f, 40.f, 11.f);
        ConfirmButton->SetStyle(BtnStyle);
    }
    UTextBlock* ConfirmText = WidgetTree->ConstructWidget<UTextBlock>();
    ConfirmText->SetText(FText::FromString(TEXT(">> CONFIRMER LE CHARGEMENT")));
    ConfirmText->SetFont(BtnFont);
    ConfirmText->SetJustification(ETextJustify::Center);
    ConfirmText->SetColorAndOpacity(FSlateColor(C_White));
    ConfirmButton->AddChild(ConfirmText);
    ConfirmButton->OnClicked.AddDynamic(this, &ULoadoutWidget::ConfirmLoadout);
    BindButtonSounds(ConfirmButton);
    VBox->AddChildToVerticalBox(ConfirmButton)->SetHorizontalAlignment(HAlign_Center);

    if (AvailableWeapons.Num() >= 2)
    {
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
  PlayClickSound();
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
