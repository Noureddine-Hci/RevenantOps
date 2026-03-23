// Copyright RevenantOps. All Rights Reserved.

#include "TitleScreenWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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
  // Actual level transition handled in Blueprint (to avoid MCP level-change crash)
}

void UTitleScreenWidget::OnQuitClicked() {
  UKismetSystemLibrary::QuitGame(
      GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
