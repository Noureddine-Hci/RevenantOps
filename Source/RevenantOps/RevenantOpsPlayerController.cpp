// Copyright Epic Games, Inc. All Rights Reserved.


#include "RevenantOpsPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "RevenantOps.h"
#include "RevenantOpsCharacter.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UI/RevenantOpsHUD.h"
#include "UI/TitleScreenWidget.h"
#include "UI/LoadoutWidget.h"
#include "UI/GameOverWidget.h"
#include "UI/LeaderboardWidget.h"
#include "UI/LeaderboardSaveGame.h"
#include "Gameplay/MercenairesGameState.h"
#include "WeaponBase.h"

void ARevenantOpsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		// Create and display the HUD widget
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<URevenantOpsHUD>(this, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport(0);
			}
			else
			{
				UE_LOG(LogRevenantOps, Error, TEXT("Could not create HUD widget."));
			}
		}

		// only spawn touch controls on mobile
		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);
			}
			else
			{
				UE_LOG(LogRevenantOps, Error, TEXT("Could not spawn mobile controls widget."));
			}
		}

		// Start the Mercenaires flow with the title screen
		if (TitleScreenClass)
		{
			ShowTitleScreen();
		}
	}
}

void ARevenantOpsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool ARevenantOpsPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

// =============================================================================
// MERCENAIRES GAME FLOW
// =============================================================================

void ARevenantOpsPlayerController::ShowTitleScreen() {
  ClearFlowWidgets();

  if (TitleScreenClass) {
    TitleScreenWidget = CreateWidget<UTitleScreenWidget>(this, TitleScreenClass);
    if (TitleScreenWidget) {
      TitleScreenWidget->AddToViewport(10);
      SetShowMouseCursor(true);
      SetInputMode(FInputModeUIOnly());
    }
  }
}

void ARevenantOpsPlayerController::ShowLoadoutScreen() {
  ClearFlowWidgets();

  if (LoadoutWidgetClass) {
    LoadoutWidgetInstance =
        CreateWidget<ULoadoutWidget>(this, LoadoutWidgetClass);
    if (LoadoutWidgetInstance) {
      LoadoutWidgetInstance->PopulateFromClasses(AvailableWeaponClasses);
      LoadoutWidgetInstance->OnLoadoutConfirmed.AddDynamic(
          this, &ARevenantOpsPlayerController::OnLoadoutConfirmed);
      LoadoutWidgetInstance->AddToViewport(10);
      SetShowMouseCursor(true);
      SetInputMode(FInputModeUIOnly());
    }
  }
}

void ARevenantOpsPlayerController::OnLoadoutConfirmed(
    TSubclassOf<AWeaponBase> Primary, TSubclassOf<AWeaponBase> Secondary) {
  // Apply loadout to character
  if (ARevenantOpsCharacter *MercChar =
          Cast<ARevenantOpsCharacter>(GetPawn())) {
    TArray<TSubclassOf<AWeaponBase>> Loadout;
    Loadout.Add(Primary);
    Loadout.Add(Secondary);
    MercChar->SetDefaultWeaponClasses(Loadout);
    MercChar->SpawnDefaultWeapons();
  }

  StartMercenairesMatch();
}

void ARevenantOpsPlayerController::StartMercenairesMatch() {
  ClearFlowWidgets();

  // Show HUD
  if (!HUDWidget && HUDWidgetClass) {
    HUDWidget = CreateWidget<URevenantOpsHUD>(this, HUDWidgetClass);
  }
  if (HUDWidget && !HUDWidget->IsInViewport()) {
    HUDWidget->AddToViewport(0);
  }

  // Switch to game input
  SetShowMouseCursor(false);
  SetInputMode(FInputModeGameOnly());

  // Start the match
  if (AMercenairesGameState *GS =
          GetWorld()->GetGameState<AMercenairesGameState>()) {
    GS->OnMatchStateChanged.AddDynamic(
        this, &ARevenantOpsPlayerController::OnMatchEnded);
    GS->StartMatch();
  }
}

void ARevenantOpsPlayerController::OnMatchEnded(bool bIsActive) {
  if (bIsActive) {
    return;
  }

  ShowGameOverScreen();
}

void ARevenantOpsPlayerController::ShowGameOverScreen() {
  // Hide HUD
  if (HUDWidget) {
    HUDWidget->RemoveFromParent();
  }

  if (GameOverWidgetClass) {
    GameOverWidgetInstance =
        CreateWidget<UGameOverWidget>(this, GameOverWidgetClass);
    if (GameOverWidgetInstance) {
      // Get match results and persist score
      if (AMercenairesGameState *GS =
              GetWorld()->GetGameState<AMercenairesGameState>()) {
        const int32 FinalScore = GS->GetCurrentScore();
        const int32 TotalKills = GS->GetTotalKills();
        const int32 BestCombo  = GS->GetBestCombo();

        ULeaderboardWidget::SaveScoreStatic(this, FinalScore, TotalKills, BestCombo);
        GameOverWidgetInstance->ShowResults(FinalScore, TotalKills, BestCombo);
      }
      GameOverWidgetInstance->AddToViewport(10);
      SetShowMouseCursor(true);
      SetInputMode(FInputModeUIOnly());
    }
  }
}

void ARevenantOpsPlayerController::ShowLeaderboard() {
  ClearFlowWidgets();

  if (LeaderboardWidgetClass) {
    LeaderboardWidgetInstance =
        CreateWidget<ULeaderboardWidget>(this, LeaderboardWidgetClass);
    if (LeaderboardWidgetInstance) {
      LeaderboardWidgetInstance->LoadScores();
      LeaderboardWidgetInstance->AddToViewport(10);
      SetShowMouseCursor(true);
      SetInputMode(FInputModeUIOnly());
    }
  }
}

void ARevenantOpsPlayerController::ClearFlowWidgets() {
  if (TitleScreenWidget) {
    TitleScreenWidget->RemoveFromParent();
    TitleScreenWidget = nullptr;
  }
  if (LoadoutWidgetInstance) {
    LoadoutWidgetInstance->RemoveFromParent();
    LoadoutWidgetInstance = nullptr;
  }
  if (GameOverWidgetInstance) {
    GameOverWidgetInstance->RemoveFromParent();
    GameOverWidgetInstance = nullptr;
  }
  if (LeaderboardWidgetInstance) {
    LeaderboardWidgetInstance->RemoveFromParent();
    LeaderboardWidgetInstance = nullptr;
  }
}
