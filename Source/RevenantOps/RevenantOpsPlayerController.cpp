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
#include "UI/InventoryWidget.h"
#include "Gameplay/InventoryItem.h"
#include "Gameplay/MercenairesGameState.h"
#include "WeaponBase.h"
#include "HealthComponent.h"
#include "AI/EnemyWaveSpawner.h"
#include "Kismet/GameplayStatics.h"

void ARevenantOpsPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// Widget creation moved to ReceivedPlayer() — safe to call CreateWidget there.
}

void ARevenantOpsPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	// ReceivedPlayer can be called more than once (seamless travel); guard it.
	if (bFlowInitialized) return;
	if (!IsLocalPlayerController()) return;

	bFlowInitialized = true;

	// Pre-create the HUD widget but do NOT add to viewport yet.
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<URevenantOpsHUD>(this, HUDWidgetClass);
		if (!HUDWidget)
		{
			UE_LOG(LogRevenantOps, Error, TEXT("Could not create HUD widget."));
		}
	}

	// Only spawn touch controls on mobile.
	if (ShouldUseTouchControls())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogRevenantOps, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}

	// Start the Mercenaires flow with the title screen.
	if (TitleScreenClass)
	{
		ShowTitleScreen();
	}
}

void ARevenantOpsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Inventory toggle — Tab key (works alongside Enhanced Input)
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this,
	                        &ARevenantOpsPlayerController::ToggleInventory);

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
    if (Primary)                        Loadout.Add(Primary);
    if (Secondary && Secondary != Primary) Loadout.Add(Secondary);
    if (Loadout.Num() > 0) {
      MercChar->SetDefaultWeaponClasses(Loadout);
      MercChar->SpawnDefaultWeapons();
    }
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

  // Bind player death to end match immediately
  if (ARevenantOpsCharacter *PlayerChar =
          Cast<ARevenantOpsCharacter>(GetPawn())) {
    if (UHealthComponent *HC =
            PlayerChar->FindComponentByClass<UHealthComponent>()) {
      HC->OnDeath.RemoveDynamic(
          this, &ARevenantOpsPlayerController::OnPlayerDied);
      HC->OnDeath.AddDynamic(
          this, &ARevenantOpsPlayerController::OnPlayerDied);
    }
  }

  // Démarrer tous les WaveSpawners du level
  TArray<AActor*> Spawners;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyWaveSpawner::StaticClass(), Spawners);
  UE_LOG(LogTemp, Warning, TEXT("[PC] StartMercenairesMatch: found %d WaveSpawner(s)"), Spawners.Num());
  for (AActor* S : Spawners)
  {
    if (AEnemyWaveSpawner* WS = Cast<AEnemyWaveSpawner>(S))
    {
      UE_LOG(LogTemp, Warning, TEXT("[PC] Calling StartEncounter on %s"), *S->GetName());
      WS->StartEncounter();
    }
  }
}

void ARevenantOpsPlayerController::OnPlayerDied(
    UHealthComponent *HealthComp, const AController *InstigatedBy,
    AActor *DamageCauser) {
  // Disable player input immediately
  if (APawn *P = GetPawn()) {
    P->DisableInput(this);
  }

  // End the match — this will trigger OnMatchEnded via the delegate
  if (AMercenairesGameState *GS =
          GetWorld()->GetGameState<AMercenairesGameState>()) {
    GS->EndMatch();
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

  // Read stats BEFORE creating the widget (GS must be read while still valid)
  int32 FinalScore = 0;
  int32 TotalKills = 0;
  int32 BestCombo  = 0;
  if (AMercenairesGameState *GS =
          GetWorld()->GetGameState<AMercenairesGameState>()) {
    FinalScore = GS->GetCurrentScore();
    TotalKills = GS->GetTotalKills();
    BestCombo  = GS->GetBestCombo();
    ULeaderboardWidget::SaveScoreStatic(this, FinalScore, TotalKills, BestCombo);
    UE_LOG(LogTemp, Warning, TEXT("[GameOver] Score=%d Kills=%d BestCombo=%d"),
        FinalScore, TotalKills, BestCombo);
  } else {
    UE_LOG(LogTemp, Error, TEXT("[GameOver] GameState null — stats will show 0!"));
  }

  if (GameOverWidgetClass) {
    GameOverWidgetInstance =
        CreateWidget<UGameOverWidget>(this, GameOverWidgetClass);
    if (GameOverWidgetInstance) {
      GameOverWidgetInstance->AddToViewport(10);
      GameOverWidgetInstance->ShowResults(FinalScore, TotalKills, BestCombo);
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

// =============================================================================
// INVENTORY
// =============================================================================

void ARevenantOpsPlayerController::ToggleInventory() {
  UE_LOG(LogRevenantOps, Warning, TEXT("[Inventory] ToggleInventory called — bInventoryOpen=%d WidgetClass=%s"),
    bInventoryOpen, InventoryWidgetClass ? *InventoryWidgetClass->GetName() : TEXT("NULL"));

  if (bInventoryOpen) {
    // Close inventory
    if (InventoryWidgetInstance) {
      InventoryWidgetInstance->RemoveFromParent();
      InventoryWidgetInstance = nullptr;
    }
    bInventoryOpen = false;

    // Restore time and input
    GetWorldSettings()->TimeDilation = 1.f;
    SetShowMouseCursor(false);
    SetInputMode(FInputModeGameOnly());

  } else {
    // Open inventory
    if (!InventoryWidgetClass) {
      UE_LOG(LogRevenantOps, Warning, TEXT("InventoryWidgetClass not set on PlayerController!"));
      return;
    }

    InventoryWidgetInstance = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
    if (!InventoryWidgetInstance) return;

    // Feed current inventory items
    if (ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(GetPawn())) {
      InventoryWidgetInstance->RefreshSlots(Char->GetInventoryItems());
    }

    // Bind use delegate
    InventoryWidgetInstance->OnItemUsed.AddDynamic(
        this, &ARevenantOpsPlayerController::OnInventoryItemUsed);
    InventoryWidgetInstance->OnClosed.AddDynamic(
        this, &ARevenantOpsPlayerController::ToggleInventory);

    InventoryWidgetInstance->AddToViewport(15);
    bInventoryOpen = true;

    // Slow time and block all game input (movement, fire, etc.)
    GetWorldSettings()->TimeDilation = 0.3f;
    SetShowMouseCursor(true);
    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
    SetInputMode(Mode);
  }
}

void ARevenantOpsPlayerController::OnInventoryItemUsed(int32 SlotIndex) {
  if (ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(GetPawn())) {
    // Check if it's a time bonus before consuming
    const TArray<FInventoryItem>& Items = Char->GetInventoryItems();
    if (Items.IsValidIndex(SlotIndex) && Items[SlotIndex].Type == EInventoryItemType::TimeBonus) {
      const float Bonus = Items[SlotIndex].TimeBonusSeconds;
      Char->UseInventoryItem(SlotIndex); // clears the slot
      // Apply time bonus to game state
      if (AMercenairesGameState* GS = GetWorld()->GetGameState<AMercenairesGameState>()) {
        GS->AddBonusTime(Bonus);
      }
    } else {
      Char->UseInventoryItem(SlotIndex);
    }

    // Refresh inventory display
    if (InventoryWidgetInstance) {
      InventoryWidgetInstance->RefreshSlots(Char->GetInventoryItems());
    }
  }
}
