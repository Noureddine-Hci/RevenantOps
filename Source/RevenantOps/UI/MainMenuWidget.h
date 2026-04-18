// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMainMenuAction);

/**
 * Main menu widget — Jouer / Options / Quitter.
 * Fully programmatic (no WBP widget placement needed).
 * Create a WBP with parent = UMainMenuWidget and assign it to the PlayerController.
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnMainMenuAction OnPlayClicked;

    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnMainMenuAction OnOptionsClicked;

    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnMainMenuAction OnQuitClicked;

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

    /** Image de fond — assigne ta texture dans les Class Defaults du WBP */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance")
    UTexture2D* BackgroundImage = nullptr;

protected:
    // Optional bind — filled by BuildDefaultUI if WBP has no manual widgets
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnPlay    = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnOptions = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnQuit    = nullptr;

private:
    void BuildDefaultUI();

    UFUNCTION() void HandlePlay();
    UFUNCTION() void HandleOptions();
    UFUNCTION() void HandleQuit();

    bool bUIBuilt = false;
};
