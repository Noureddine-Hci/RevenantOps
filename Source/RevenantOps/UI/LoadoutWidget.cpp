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

  // Auto-select first two weapons if available and not already set by BuildDefaultUI
  if (AvailableWeapons.Num() >= 2) {
    PrimaryWeaponIndex   = 0;
    SecondaryWeaponIndex = 1;
  } else {
    PrimaryWeaponIndex   = -1;
    SecondaryWeaponIndex = -1;
  }
}

void ULoadoutWidget::BuildDefaultUI() {
  if (!WidgetTree) return;
  bDefaultUIBuilt = true;

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

  UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
  UCanvasPanelSlot* VBoxSlot = Canvas->AddChildToCanvas(VBox);
  VBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
  VBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
  VBoxSlot->SetAutoSize(true);

  FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 32);
  FSlateFontInfo InfoFont  = FCoreStyle::GetDefaultFontStyle("Regular", 14);
  FSlateFontInfo BtnFont   = FCoreStyle::GetDefaultFontStyle("Regular", 18);

  // Title
  UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
  Title->SetText(FText::FromString(TEXT("CHOISISSEZ VOS ARMES")));
  Title->SetFont(TitleFont);
  Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.8f, 0.2f)));
  Title->SetJustification(ETextJustify::Center);
  UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(Title);
  TitleSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  TitleSlot->SetPadding(FMargin(0, 0, 0, 20));

  // Weapon list (informational)
  WeaponListBox = WidgetTree->ConstructWidget<UVerticalBox>();
  for (int32 i = 0; i < AvailableWeapons.Num(); ++i) {
    const FWeaponLoadoutInfo& Info = AvailableWeapons[i];

    FString NameStr = Info.WeaponName.IsEmpty()
                          ? FString::Printf(TEXT("Arme %d"), i + 1)
                          : Info.WeaponName.ToString();

    FString Line = FString::Printf(
        TEXT("%s%s  —  DMG:%.0f  ROF:%.0f  MAG:%d  RLD:%.1fs"),
        (i == 0) ? TEXT("[P] ") : (i == 1) ? TEXT("[S] ") : TEXT("    "),
        *NameStr, Info.Damage, Info.FireRate, Info.MagazineSize, Info.ReloadTime);

    UTextBlock* Entry = WidgetTree->ConstructWidget<UTextBlock>();
    Entry->SetText(FText::FromString(Line));
    Entry->SetFont(InfoFont);
    Entry->SetJustification(ETextJustify::Center);

    // Highlight first two (auto-selected)
    if (i == 0 || i == 1) {
      Entry->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.f, 0.2f)));
    }

    UVerticalBoxSlot* EntrySlot = WeaponListBox->AddChildToVerticalBox(Entry);
    EntrySlot->SetPadding(FMargin(0, 3, 0, 3));
  }
  UVerticalBoxSlot* ListSlot = VBox->AddChildToVerticalBox(WeaponListBox);
  ListSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  ListSlot->SetPadding(FMargin(0, 0, 0, 15));

  // Auto-select first two weapons
  if (AvailableWeapons.Num() >= 2) {
    PrimaryWeaponIndex = 0;
    SecondaryWeaponIndex = 1;
  }

  // Selection status text
  SelectionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SelectionText"));
  FString PriName = (PrimaryWeaponIndex >= 0) ? AvailableWeapons[PrimaryWeaponIndex].WeaponName.ToString() : TEXT("---");
  FString SecName = (SecondaryWeaponIndex >= 0) ? AvailableWeapons[SecondaryWeaponIndex].WeaponName.ToString() : TEXT("---");
  SelectionText->SetText(FText::FromString(
      FString::Printf(TEXT("Primaire: %s | Secondaire: %s"), *PriName, *SecName)));
  SelectionText->SetFont(InfoFont);
  SelectionText->SetJustification(ETextJustify::Center);
  SelectionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 1.f, 0.5f)));
  UVerticalBoxSlot* SelSlot = VBox->AddChildToVerticalBox(SelectionText);
  SelSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
  SelSlot->SetPadding(FMargin(0, 0, 0, 20));

  // Confirm button
  ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ConfirmButton"));
  UTextBlock* ConfirmText = WidgetTree->ConstructWidget<UTextBlock>();
  ConfirmText->SetText(FText::FromString(TEXT("CONFIRMER")));
  ConfirmText->SetFont(BtnFont);
  ConfirmText->SetJustification(ETextJustify::Center);
  ConfirmButton->AddChild(ConfirmText);
  ConfirmButton->OnClicked.AddDynamic(this, &ULoadoutWidget::ConfirmLoadout);
  UVerticalBoxSlot* ConfSlot = VBox->AddChildToVerticalBox(ConfirmButton);
  ConfSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
}

void ULoadoutWidget::RefreshWeaponButtons() {
  if (SelectionText) {
    FString PriName = (PrimaryWeaponIndex >= 0 && PrimaryWeaponIndex < AvailableWeapons.Num())
                          ? AvailableWeapons[PrimaryWeaponIndex].WeaponName.ToString()
                          : TEXT("---");
    FString SecName = (SecondaryWeaponIndex >= 0 && SecondaryWeaponIndex < AvailableWeapons.Num())
                          ? AvailableWeapons[SecondaryWeaponIndex].WeaponName.ToString()
                          : TEXT("---");
    SelectionText->SetText(FText::FromString(
        FString::Printf(TEXT("Primaire: %s | Secondaire: %s"), *PriName, *SecName)));
  }
}

void ULoadoutWidget::SelectWeapon(int32 WeaponIndex, int32 SlotIndex) {
  if (WeaponIndex < 0 || WeaponIndex >= AvailableWeapons.Num()) {
    return;
  }

  if (SlotIndex == 0) {
    if (WeaponIndex == SecondaryWeaponIndex) {
      SecondaryWeaponIndex = -1;
    }
    PrimaryWeaponIndex = WeaponIndex;
  } else if (SlotIndex == 1) {
    if (WeaponIndex == PrimaryWeaponIndex) {
      PrimaryWeaponIndex = -1;
    }
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
    // Auto-pick first two weapons (fallback if selection indices are stale)
    Primary   = AvailableWeapons[0].WeaponClass;
    Secondary = AvailableWeapons[1].WeaponClass;
  } else if (AvailableWeapons.Num() == 1) {
    Primary   = AvailableWeapons[0].WeaponClass;
    Secondary = AvailableWeapons[0].WeaponClass;
  }
  // else: Primary/Secondary = nullptr — match starts without weapons (demo fallback)

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
    if (!WeapClass) {
      continue;
    }

    const AWeaponBase *CDO = WeapClass->GetDefaultObject<AWeaponBase>();
    if (!CDO) {
      continue;
    }

    FWeaponLoadoutInfo Info;
    Info.WeaponClass = WeapClass;
    Info.WeaponName = CDO->GetWeaponName();
    Info.Damage = CDO->GetBaseDamage();
    Info.FireRate = CDO->GetFireRate();
    Info.MagazineSize = CDO->GetMagazineSize();
    Info.ReloadTime = CDO->GetReloadTime();
    Info.WeaponIcon = nullptr;

    AvailableWeapons.Add(Info);
  }

  // Auto-select first two weapons — NativeConstruct runs before PopulateFromClasses
  // so indices are -1 at that point; fix them here.
  if (AvailableWeapons.Num() >= 2 && PrimaryWeaponIndex < 0) {
    PrimaryWeaponIndex   = 0;
    SecondaryWeaponIndex = 1;
  }

  // Rebuild the weapon list UI (BuildDefaultUI ran when AvailableWeapons was empty)
  if (WeaponListBox) {
    WeaponListBox->ClearChildren();
    FSlateFontInfo InfoFont = FCoreStyle::GetDefaultFontStyle("Regular", 14);
    for (int32 i = 0; i < AvailableWeapons.Num(); ++i) {
      const FWeaponLoadoutInfo &Info = AvailableWeapons[i];
      FString NameStr = Info.WeaponName.IsEmpty()
                            ? FString::Printf(TEXT("Arme %d"), i + 1)
                            : Info.WeaponName.ToString();
      FString Line = FString::Printf(
          TEXT("%s%s  —  DMG:%.0f  ROF:%.0f  MAG:%d  RLD:%.1fs"),
          (i == PrimaryWeaponIndex) ? TEXT("[P] ") : (i == SecondaryWeaponIndex) ? TEXT("[S] ") : TEXT("    "),
          *NameStr, Info.Damage, Info.FireRate, Info.MagazineSize, Info.ReloadTime);
      UTextBlock *Entry = NewObject<UTextBlock>(WeaponListBox);
      Entry->SetText(FText::FromString(Line));
      Entry->SetFont(InfoFont);
      Entry->SetJustification(ETextJustify::Center);
      if (i == PrimaryWeaponIndex || i == SecondaryWeaponIndex) {
        Entry->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 1.f, 0.2f)));
      }
      UVerticalBoxSlot *EntrySlot = WeaponListBox->AddChildToVerticalBox(Entry);
      EntrySlot->SetPadding(FMargin(0, 3, 0, 3));
    }
  }
  RefreshWeaponButtons();
}
