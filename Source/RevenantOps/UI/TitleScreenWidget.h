// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/MenuWidgetBase.h"
#include "TitleScreenWidget.generated.h"

class UButton;
class UTextBlock;
class ARevenantOpsPlayerController;

/**
 *  Title screen widget.
 *  Displays "Play" and "Quit" buttons.
 *  Bind UButton widgets named PlayButton and QuitButton in the Blueprint.
 */
UCLASS(abstract, Blueprintable)
class UTitleScreenWidget : public UMenuWidgetBase {
  GENERATED_BODY()

public:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;

protected:
  UPROPERTY(meta = (BindWidgetOptional))
  UButton *PlayButton;

  UPROPERTY(meta = (BindWidgetOptional))
  UButton *QuitButton;

  UPROPERTY(meta = (BindWidgetOptional))
  UButton *OptionsButton;

  UFUNCTION()
  void OnPlayClicked();

  UFUNCTION()
  void OnQuitClicked();

  UFUNCTION()
  void OnOptionsClicked();

  /** BP hook for transition effects before starting */
  UFUNCTION(BlueprintImplementableEvent, Category = "UI",
            meta = (DisplayName = "On Play Pressed"))
  void BP_OnPlayPressed();

private:
  /** Auto-build a functional UI when designer layout is empty */
  void BuildDefaultUI();
};
