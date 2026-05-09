// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/InventoryItem.h"
#include "InputAction.h"
#include "InventoryWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UImage;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemUsed,     int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemDropped,  int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemsCombined, int32, SlotA, int32, SlotB);
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

  /** Relit les touches actuelles depuis Enhanced Input et met à jour les labels des boutons */
  UFUNCTION(BlueprintCallable, Category = "Inventory")
  void RefreshKeyHints();

  // ── InputActions à assigner dans le WBP (Class Defaults) ─────────────────
  /** Action "utiliser/équiper" — assigner IA_Use ou équivalent dans le BP */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Input")
  TObjectPtr<UInputAction> UseAction;

  /** Action "jeter" */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Input")
  TObjectPtr<UInputAction> DropAction;

  /** Action "fermer inventaire" */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Input")
  TObjectPtr<UInputAction> CloseAction;

  UPROPERTY(BlueprintAssignable, Category = "Inventory")
  FOnInventoryItemUsed OnItemUsed;

  UPROPERTY(BlueprintAssignable, Category = "Inventory")
  FOnInventoryItemDropped OnItemDropped;

  UPROPERTY(BlueprintAssignable, Category = "Inventory")
  FOnInventoryItemsCombined OnItemCombined;

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

  // Mode combine RE5
  bool  bCombineMode        = false;
  int32 CombineSourceIndex  = -1;

  // Slot UI references (9 each)
  UPROPERTY() TArray<UBorder*>    SlotBorders;
  UPROPERTY() TArray<UImage*>     SlotImages;    // icone ou couleur de type
  UPROPERTY() TArray<UTextBlock*> SlotQtyTexts;

  // Panneau info compact (bas du widget)
  UPROPERTY() UTextBlock* InfoNameText    = nullptr;  // nom + quantité
  UPROPERTY() UBorder*    BtnUse         = nullptr;  // [E] Équiper/Utiliser
  UPROPERTY() UTextBlock* BtnUseLabel    = nullptr;  // label dynamique
  UPROPERTY() UBorder*    BtnCombine     = nullptr;  // [C] Combiner
  UPROPERTY() UBorder*    BtnDrop        = nullptr;  // [X] Jeter
  UPROPERTY() UTextBlock* InfoHintText   = nullptr;  // [Tab] Fermer

  bool bUIBuilt = false;

  // Labels de touches mis en cache par RefreshKeyHints() — utilisés dans UpdateInfoPanel()
  FString CachedKeyUse   = TEXT("[E]");
  FString CachedKeyDrop  = TEXT("[X]");
  FString CachedKeyClose = TEXT("[Tab]");
};
