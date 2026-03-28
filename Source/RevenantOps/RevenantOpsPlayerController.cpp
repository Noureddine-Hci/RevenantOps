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
#include "AI/EnemyWaveSpawner.h"
#include "Kismet/GameplayStatics.h"

void ARevenantOpsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		// Pre-create the HUD widget but do NOT add to viewport yet.
		// It will be shown by StartMercenairesMatch() once the match begins.
		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<URevenantOpsHUD>(this, HUDWidgetClass);
			if (!HUDWidget)
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
  bLoadoutConfirmed = false;

  // If no weapon classes configured in BP, load defaults from known content paths
  if (AvailableWeaponClasses.IsEmpty())
  {
    static const TCHAR* DefaultPaths[] = {
      TEXT("/Game/Mercenaires/Weapons/BP_Pistol.BP_Pistol_C"),
      TEXT("/Game/Mercenaires/Weapons/BP_AssaultRifle.BP_AssaultRifle_C"),
      TEXT("/Game/Mercenaires/Weapons/BP_SMG.BP_SMG_C"),
      TEXT("/Game/Mercenaires/Weapons/BP_Shotgun.BP_Shotgun_C"),
      TEXT("/Game/Mercenaires/Weapons/BP_Sniper.BP_Sniper_C"),
    };
    for (const TCHAR* Path : DefaultPaths)
    {
      if (UClass* WC = LoadClass<AWeaponBase>(nullptr, Path))
      {
        AvailableWeaponClasses.Add(WC);
      }
    }
  }

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
  if (bLoadoutConfirmed) {
    return;
  }
  bLoadoutConfirmed = true;

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
    // Unbind first to prevent double-bind on replay
    GS->OnMatchStateChanged.RemoveDynamic(
        this, &ARevenantOpsPlayerController::OnMatchEnded);
    GS->OnMatchStateChanged.AddDynamic(
        this, &ARevenantOpsPlayerController::OnMatchEnded);
    GS->StartMatch();
  }

  // Démarrer tous les WaveSpawners du level
  TArray<AActor*> Spawners;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyWaveSpawner::StaticClass(), Spawners);
  for (AActor* S : Spawners)
  {
    if (AEnemyWaveSpawner* WS = Cast<AEnemyWaveSpawner>(S))
    {
      WS->StartEncounter();
    }
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
