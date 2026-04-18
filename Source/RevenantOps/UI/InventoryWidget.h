// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/InventoryItem.h"
#include "InventoryWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UImage;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemUsed, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryClose);

/**
 *  Inventaire style Resident Evil 5 — 9 slots (3x3), navigation WASD/fleches.
 *  Tab ou Echap pour fermer. E pour utiliser l'item selectionne.
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API UInventoryWidget : public UUserWidget {
  GENERATED_BODY()

public:
  static const int32 SLOT_COUNT = 9;
  static const int32 COLS       = 3;

  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;
  virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry,
                                  const FKeyEvent& InKeyEvent) override;
  virtual bool NativeSupportsKeyboardFocus() const override { return true; }

  /** Met a jour l'affichage depuis un tableau de 9 items */
  void RefreshSlots(const TArray<FInventoryItem>& Items);

  UPROPERTY(BlueprintAssignable, Category = "Inventory")
  FOnInventoryItemUsed OnItemUsed;

  UPROPERTY(BlueprintAssignable, Category = "Inventory")
  FOnInventoryClose OnClosed;

private:
  void BuildDefaultUI();
  void MoveSelection(int32 DeltaRow, int32 DeltaCol);
  void UpdateVisuals();
  void UpdateInfoPanel();

  // Color helpers per item type
  static FLinearColor SlotColor(EInventoryItemType Type);
  static FLinearColor BorderColor(EInventoryItemType Type);

  int32 SelectedIndex = 0;
  TArray<FInventoryItem> CachedItems;

  // Slot UI references (9 each)
  UPROPERTY() TArray<UBorder*>    SlotBorders;
  UPROPERTY() TArray<UImage*>     SlotImages;    // icone ou couleur de type
  UPROPERTY() TArray<UTextBlock*> SlotQtyTexts;

  // Info panel
  UPROPERTY() UImage*     InfoIconImage = nullptr;  // grande icone a gauche
  UPROPERTY() UTextBlock* InfoNameText  = nullptr;
  UPROPERTY() UTextBlock* InfoDescText  = nullptr;
  UPROPERTY() UTextBlock* InfoHintText  = nullptr;

  bool bUIBuilt = false;
};
