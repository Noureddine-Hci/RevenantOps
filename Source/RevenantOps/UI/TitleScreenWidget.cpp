// Copyright RevenantOps. All Rights Reserved.

#include "TitleScreenWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RevenantOpsPlayerController.h"

void UTitleScreenWidget::NativeConstruct() {
  Super::NativeConstruct();

  if (PlayButton) {
    PlayButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnPlayClicked);
  }

  if (QuitButton) {
    QuitButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnQuitClicked);
  }
}

void UTitleScreenWidget::OnPlayClicked() {
  BP_OnPlayPressed();

  if (ARevenantOpsPlayerController* PC =
          Cast<ARevenantOpsPlayerController>(GetOwningPlayer()))
  {
    PC->ShowLoadoutScreen();
  }
}

void UTitleScreenWidget::OnQuitClicked() {
  UKismetSystemLibrary::QuitGame(
      GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
