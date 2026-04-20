// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MenuTypes.h"
#include "CharacterSelectWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UBorder;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterChosen, FCharacterInfo, CharacterInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterSelectBack);

UCLASS(Blueprintable)
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
    void SelectCharacter(int32 Index);
    void RefreshCarousel();
    void RefreshInfo();

    UFUNCTION() void HandleBack();
    UFUNCTION() void HandlePrev();
    UFUNCTION() void HandleNext();
    UFUNCTION() void HandleConfirm();

    TArray<FCharacterInfo> CachedCharacters;
    int32 SelectedIndex = 0;

    UPROPERTY() UImage*         PortraitImage   = nullptr;
    UPROPERTY() UHorizontalBox* CarouselBox     = nullptr;
    UPROPERTY() UTextBlock*     CharNameText    = nullptr;
    UPROPERTY() UButton*        BtnPrev         = nullptr;
    UPROPERTY() UButton*        BtnNext         = nullptr;
    UPROPERTY() UButton*        BtnConfirm      = nullptr;
    UPROPERTY() UButton*        BtnBack         = nullptr;

    bool bUIBuilt = false;
};
