// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MenuTypes.h"
#include "LevelSelectWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UBorder;
class UImage;
class UMenuCardWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChosen, FLevelInfo, LevelInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelSelectBack);

/**
 * Écran de sélection de niveau — style sombre militaire.
 * Carousel horizontal + leaderboard par niveau en bas.
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

    void PopulateLevels(const TArray<FLevelInfo>& Levels);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

private:
    void BuildDefaultUI();
    void SelectLevel(int32 Index);
    void RefreshCarousel();
    void RefreshLeaderboard();

    UFUNCTION() void HandleBack();
    UFUNCTION() void HandlePrev();
    UFUNCTION() void HandleNext();
    UFUNCTION() void HandleConfirm();

    TArray<FLevelInfo> CachedLevels;
    int32 SelectedIndex = 0;

    // Carousel
    UPROPERTY() UHorizontalBox* CarouselBox     = nullptr;
    UPROPERTY() UTextBlock*     LevelNameText   = nullptr;
    UPROPERTY() UTextBlock*     BestScoreText   = nullptr;
    UPROPERTY() UButton*        BtnPrev         = nullptr;
    UPROPERTY() UButton*        BtnNext         = nullptr;
    UPROPERTY() UButton*        BtnConfirm      = nullptr;
    UPROPERTY() UButton*        BtnBack         = nullptr;

    // Leaderboard
    UPROPERTY() UVerticalBox*   LeaderboardBox  = nullptr;
    UPROPERTY() UTextBlock*     LbTitle         = nullptr;

    bool bUIBuilt = false;
};
