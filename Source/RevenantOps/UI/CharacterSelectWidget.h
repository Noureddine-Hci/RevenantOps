// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MenuTypes.h"
#include "CharacterSelectWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UMenuCardWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterChosen, FCharacterInfo, CharacterInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterSelectBack);

/**
 * Character selection screen.
 * Call PopulateCharacters() after creating the widget.
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API UCharacterSelectWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnCharacterChosen OnCharacterChosen;

    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnCharacterSelectBack OnBackClicked;

    void PopulateCharacters(const TArray<FCharacterInfo>& Characters);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

private:
    void BuildDefaultUI();
    void HandleCardClicked(int32 Index);

    UFUNCTION() void HandleBack();

    TArray<FCharacterInfo> CachedCharacters;

    UPROPERTY() UScrollBox*              CardContainer = nullptr;
    UPROPERTY() UButton*                 BtnBack       = nullptr;
    UPROPERTY() TArray<UMenuCardWidget*> Cards;

    bool bUIBuilt = false;
};
