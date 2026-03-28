// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleScreenWidget.generated.h"

class UButton;
class UTextBlock;

/**
 *  Title screen widget.
 *  Displays "Play" and "Quit" buttons.
 *  Bind UButton widgets named PlayButton and QuitButton in the Blueprint.
 */
UCLASS(abstract, Blueprintable)
class UTitleScreenWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;

protected:
  UPROPERTY(meta = (BindWidgetOptional))
  UButton *PlayButton;

  UPROPERTY(meta = (BindWidgetOptional))
  UButton *QuitButton;

  UFUNCTION()
  void OnPlayClicked();

  UFUNCTION()
  void OnQuitClicked();

  /** BP hook for transition effects before starting */
  UFUNCTION(BlueprintImplementableEvent, Category = "UI",
            meta = (DisplayName = "On Play Pressed"))
  void BP_OnPlayPressed();

private:
  /** Auto-build a functional UI when designer layout is empty */
  void BuildDefaultUI();
};
