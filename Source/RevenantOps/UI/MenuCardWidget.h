// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuCardWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UBorder;

/**
 * Non-dynamic delegate — supports AddUObject/AddLambda without needing UFUNCTION.
 * Used internally so LevelSelectWidget / CharacterSelectWidget can bind per-index callbacks.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMenuCardClicked, int32 /*CardIndex*/);

/**
 * Reusable selection card used by both LevelSelectWidget and CharacterSelectWidget.
 * Displays a thumbnail image, a label, and a select button.
 * Fully programmatic — no WBP needed.
 */
UCLASS(Blueprintable)
class REVENANTOPS_API UMenuCardWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Fired when the card is clicked. Passes back the CardIndex set via Setup(). */
    FOnMenuCardClicked OnCardClicked;

    /** Configure the card before adding it to the viewport */
    void Setup(int32 InIndex, const FText& InName, UTexture2D* InThumbnail);

    /** Visually highlight (selected) or de-highlight this card */
    void SetHighlighted(bool bHighlight);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

private:
    void BuildDefaultUI();

    UFUNCTION() void HandleClicked();

    int32       CardIndex     = 0;
    FText       CardName;
    UTexture2D* CardThumbnail = nullptr;

    UPROPERTY() UBorder*    RootBorder   = nullptr;
    UPROPERTY() UImage*     ThumbnailImg = nullptr;
    UPROPERTY() UTextBlock* NameText     = nullptr;
    UPROPERTY() UButton*    SelectButton = nullptr;

    bool bUIBuilt = false;
};
