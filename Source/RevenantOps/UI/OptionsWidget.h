// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/MenuWidgetBase.h"
#include "UI/MenuTypes.h"
#include "OptionsWidget.generated.h"

class UVerticalBox;
class UButton;
class UTextBlock;
class UBorder;
class UOptionsWidget;

/** Helper UObject so each "Changer" button can bind to its own UFUNCTION with its row index */
UCLASS()
class REVENANTOPS_API UKeyBindButtonHandler : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() UOptionsWidget* Parent = nullptr;
    int32 Index = -1;
    UFUNCTION() void OnClicked();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOptionsBack);

/**
 * Options menu — key rebinding.
 *
 * To enable live rebinding:
 *   1. Project Settings → Enhanced Input → Enable User Settings = true
 *   2. Each Input Action → Player Mappable Key Options → Name = your MappingName
 *
 * Fill AvailableRebinds on BP_ThirdPersonPlayerController before showing this widget.
 * If User Settings aren't enabled, rows are shown read-only with DefaultKey.
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API UOptionsWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnOptionsBack OnBackClicked;

    /** Populate rows — call before AddToViewport */
    void PopulateBindings(const TArray<FKeyRebindEntry>& Bindings);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
    virtual bool NativeSupportsKeyboardFocus() const override { return true; }

    /** Appelé par le PlayerController avant AddToViewport */
    void SetIMC(class UInputMappingContext* InIMC);

    void Tick_ListenForKey(float DeltaTime);

public:
    void StartListening(int32 Index);

private:
    void BuildDefaultUI();
    void RefreshRow(int32 Index);
    void StopListening(bool bCancelled);

    UFUNCTION() void HandleBack();

    // Each row: DisplayName | CurrentKey | "Changer" button
    struct FBindingRow
    {
        UPROPERTY() UTextBlock* KeyText   = nullptr;
        UPROPERTY() UButton*    ChangeBtn = nullptr;
        UPROPERTY() UTextBlock* ChangeLbl = nullptr;
    };

    TArray<FKeyRebindEntry>          CachedBindings;
    TArray<FBindingRow>              Rows;
    UPROPERTY() TArray<UKeyBindButtonHandler*> BtnHandlers;

    UPROPERTY() UVerticalBox* RowContainer = nullptr;
    UPROPERTY() UButton*      BtnBack      = nullptr;

    UPROPERTY() class UInputMappingContext* IMC = nullptr;

    bool  bListening      = false;
    int32 ListeningIndex  = -1;
    FTimerHandle ListenTimerHandle;

    bool bUIBuilt = false;

    void ApplyKeyToIMC(int32 Index, const FKey& NewKey);
    void LoadAndApplySavedBindings();
};
