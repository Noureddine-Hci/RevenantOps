// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MenuTypes.h"
#include "LevelSelectWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UMenuCardWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChosen, FLevelInfo, LevelInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelSelectBack);

/**
 * Level selection screen.
 * Call PopulateLevels() after creating the widget (before AddToViewport).
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API ULevelSelectWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnLevelChosen OnLevelChosen;

    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnLevelSelectBack OnBackClicked;

    /** Populate the grid with level cards — call before AddToViewport */
    void PopulateLevels(const TArray<FLevelInfo>& Levels);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

private:
    void BuildDefaultUI();
    void HandleCardClicked(int32 Index);

    UFUNCTION() void HandleBack();

    TArray<FLevelInfo> CachedLevels;

    UPROPERTY() UScrollBox*              CardContainer = nullptr;
    UPROPERTY() UButton*                 BtnBack       = nullptr;
    UPROPERTY() TArray<UMenuCardWidget*> Cards;

    bool bUIBuilt = false;
};
