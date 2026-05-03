// Copyright Epic Games, Inc. All Rights Reserved.


#include "RevenantOpsPlayerController.h"
#include "Components/AudioComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "RevenantOps.h"
#include "RevenantOpsCharacter.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UI/RevenantOpsHUD.h"
#include "UI/TitleScreenWidget.h"
#include "UI/LevelSelectWidget.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/OptionsWidget.h"
#include "UI/LoadoutWidget.h"
#include "UI/GameOverWidget.h"
#include "UI/LeaderboardWidget.h"
#include "UI/LeaderboardSaveGame.h"
#include "UI/InventoryWidget.h"
#include "UI/MenuWidgetBase.h"
#include "Gameplay/InventoryItem.h"
#include "Gameplay/MercenairesGameState.h"
#include "WeaponBase.h"
#include "HealthComponent.h"
#include "AI/EnemyWaveSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/RevenantOpsGameInstance.h"

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

	// Si un match est en attente (joueur vient du menu principal), on lance directement.
	// Sinon on affiche le title screen (on est dans Lvl_MainMenu).
	URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance());
	if (GI && GI->bPendingMatchStart)
	{
		GI->bPendingMatchStart = false;
		// Délai d'une frame pour laisser le niveau finir son initialisation
		FTimerHandle TempHandle;
		GetWorld()->GetTimerManager().SetTimer(TempHandle,
			FTimerDelegate::CreateUObject(this, &ARevenantOpsPlayerController::StartMercenairesMatch),
			0.1f, false);
	}
	else if (TitleScreenClass)
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

void ARevenantOpsPlayerController::DoTransition(TFunction<void()> Fn)
{
    if (UMenuWidgetBase* W = ActiveMenu.Get())
        W->FadeOutThen(0.15f, MoveTemp(Fn));
    else
        Fn();
    ActiveMenu.Reset();
}

void ARevenantOpsPlayerController::ShowTitleScreen() {
  DoTransition([this]()
  {
    if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance()))
      GI->bPendingMatchStart = false;

    ClearFlowWidgets();

    if (TitleScreenClass) {
      TitleScreenWidget = CreateWidget<UTitleScreenWidget>(this, TitleScreenClass);
      if (TitleScreenWidget) {
        TitleScreenWidget->AddToViewport(10);
        ActiveMenu = TitleScreenWidget.Get();
        SetShowMouseCursor(true);
        SetInputMode(FInputModeUIOnly());
      }
    }
  });
}

void ARevenantOpsPlayerController::ShowOptionsScreen() {
  DoTransition([this]()
  {
    ClearFlowWidgets();
    if (!OptionsWidgetClass) return;
    OptionsWidgetInstance = CreateWidget<UOptionsWidget>(this, OptionsWidgetClass);
    if (OptionsWidgetInstance) {
      OptionsWidgetInstance->SetIMC(DefaultMappingContext);
      OptionsWidgetInstance->PopulateBindings(AvailableRebinds);
      OptionsWidgetInstance->OnBackClicked.AddDynamic(
          this, &ARevenantOpsPlayerController::OnOptionsBack);
      OptionsWidgetInstance->AddToViewport(10);
      ActiveMenu = OptionsWidgetInstance.Get();
      SetShowMouseCursor(true);
      SetInputMode(FInputModeUIOnly());
    }
  });
}

void ARevenantOpsPlayerController::OnOptionsBack() {
  ShowTitleScreen();
}

void ARevenantOpsPlayerController::ShowLevelSelectScreen() {
  DoTransition([this]()
  {
    ClearFlowWidgets();
    if (AvailableLevels.IsEmpty()) {
      FLevelInfo DefaultLevel;
      DefaultLevel.DisplayName = FText::FromString(TEXT("The Compound"));
      DefaultLevel.MapName     = FName(TEXT("Lvl_ThirdPerson"));
      AvailableLevels.Add(DefaultLevel);
    }
    if (!LevelSelectWidgetClass) { ShowCharacterSelectScreen(); return; }
    LevelSelectWidgetInstance = CreateWidget<ULevelSelectWidget>(this, LevelSelectWidgetClass);
    if (LevelSelectWidgetInstance) {
      LevelSelectWidgetInstance->PopulateLevels(AvailableLevels);
      LevelSelectWidgetInstance->OnLevelChosen.AddDynamic(
          this, &ARevenantOpsPlayerController::OnLevelChosen);
      LevelSelectWidgetInstance->OnBackClicked.AddDynamic(
          this, &ARevenantOpsPlayerController::OnLevelSelectBack);
      LevelSelectWidgetInstance->AddToViewport(10);
      ActiveMenu = LevelSelectWidgetInstance.Get();
      SetShowMouseCursor(true);
      FInputModeUIOnly InputMode;
      InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
      SetInputMode(InputMode);
    }
  });
}

void ARevenantOpsPlayerController::OnLevelChosen(FLevelInfo LevelInfo) {
  // Stocker le niveau choisi (GameInstance) puis aller à CharacterSelect
  if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance())) {
    GI->PendingLevel = LevelInfo;
  }
  ShowCharacterSelectScreen();
}

void ARevenantOpsPlayerController::OnLevelSelectBack() {
  ShowTitleScreen();
}

void ARevenantOpsPlayerController::ShowCharacterSelectScreen() {
  DoTransition([this]()
  {
    ClearFlowWidgets();
    if (AvailableCharacters.IsEmpty()) {
      FCharacterInfo DefaultChar;
      DefaultChar.DisplayName = FText::FromString(TEXT("Mercenaire"));
      AvailableCharacters.Add(DefaultChar);
    }
    if (!CharacterSelectWidgetClass) { ShowLoadoutScreen(); return; }
    CharacterSelectWidgetInstance = CreateWidget<UCharacterSelectWidget>(this, CharacterSelectWidgetClass);
    if (CharacterSelectWidgetInstance) {
      CharacterSelectWidgetInstance->PopulateCharacters(AvailableCharacters);
      CharacterSelectWidgetInstance->OnCharacterChosen.AddDynamic(
          this, &ARevenantOpsPlayerController::OnCharacterChosen);
      CharacterSelectWidgetInstance->OnBackClicked.AddDynamic(
          this, &ARevenantOpsPlayerController::OnCharacterSelectBack);
      CharacterSelectWidgetInstance->AddToViewport(10);
      ActiveMenu = CharacterSelectWidgetInstance.Get();
      SetShowMouseCursor(true);
      SetInputMode(FInputModeUIOnly());
    }
  });
}

void ARevenantOpsPlayerController::OnCharacterChosen(FCharacterInfo CharacterInfo) {
  if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance())) {
    GI->PendingCharacter = CharacterInfo;
    // Pré-remplir les armes du loadout depuis l'inventaire par défaut du personnage
    GI->PendingPrimaryWeapon   = nullptr;
    GI->PendingSecondaryWeapon = nullptr;
    for (const FInventoryItem& Item : CharacterInfo.DefaultInventory)
    {
      if (Item.Type == EInventoryItemType::Weapon && Item.WeaponClass)
      {
        if (!GI->PendingPrimaryWeapon)        GI->PendingPrimaryWeapon   = Item.WeaponClass;
        else if (!GI->PendingSecondaryWeapon) GI->PendingSecondaryWeapon = Item.WeaponClass;
      }
    }
  }
  // Charger le niveau directement (pas d'écran de loadout)
  bLoadoutConfirmed = true;
  if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance()))
  {
    GI->bPendingMatchStart = true;
    FName LevelName = GI->PendingLevel.MapName.IsNone() ? FName("Lvl_ThirdPerson") : GI->PendingLevel.MapName;
    ClearFlowWidgets();
    UGameplayStatics::OpenLevel(this, LevelName);
  }
}

void ARevenantOpsPlayerController::OnCharacterSelectBack() {
  ShowLevelSelectScreen();
}

void ARevenantOpsPlayerController::ShowLoadoutScreen() {
  DoTransition([this]()
  {
    ClearFlowWidgets();
    bLoadoutConfirmed = false;

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
        if (UClass* WC = LoadClass<AWeaponBase>(nullptr, Path))
          AvailableWeaponClasses.Add(WC);
    }

    if (LoadoutWidgetClass) {
      LoadoutWidgetInstance = CreateWidget<ULoadoutWidget>(this, LoadoutWidgetClass);
      if (LoadoutWidgetInstance) {
        LoadoutWidgetInstance->PopulateFromClasses(AvailableWeaponClasses);
        LoadoutWidgetInstance->OnLoadoutConfirmed.AddDynamic(
            this, &ARevenantOpsPlayerController::OnLoadoutConfirmed);
        LoadoutWidgetInstance->AddToViewport(10);
        ActiveMenu = LoadoutWidgetInstance.Get();
        SetShowMouseCursor(true);
        SetInputMode(FInputModeUIOnly());
      }
    }
  }); // DoTransition
}

void ARevenantOpsPlayerController::OnLoadoutConfirmed(
    TSubclassOf<AWeaponBase> Primary, TSubclassOf<AWeaponBase> Secondary) {
  if (bLoadoutConfirmed) {
    return;
  }
  bLoadoutConfirmed = true;

  // Sauvegarde le loadout dans le GameInstance (persiste entre niveaux)
  FName LevelToLoad = TEXT("Lvl_ThirdPerson"); // fallback
  if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance()))
  {
    GI->PendingPrimaryWeapon   = Primary;
    GI->PendingSecondaryWeapon = Secondary;
    GI->bPendingMatchStart     = true;
    if (!GI->PendingLevel.MapName.IsNone())
      LevelToLoad = GI->PendingLevel.MapName;
  }

  // Charge le niveau choisi par le joueur
  UGameplayStatics::OpenLevel(this, LevelToLoad);
}

void ARevenantOpsPlayerController::RestartMatch() {
  FName LevelToLoad = TEXT("Lvl_ThirdPerson");
  if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance()))
  {
    GI->bPendingMatchStart = true;
    if (!GI->PendingLevel.MapName.IsNone())
      LevelToLoad = GI->PendingLevel.MapName;
  }
  UGameplayStatics::OpenLevel(this, LevelToLoad);
}

void ARevenantOpsPlayerController::StartMercenairesMatch() {
  // Démarre la musique in-game
  if (GameMusic && !GameMusicComponent)
  {
    GameMusicComponent = UGameplayStatics::SpawnSound2D(this, GameMusic, GameMusicVolume, 1.f, 0.f, nullptr, false, true);
  }

  ClearFlowWidgets();

  // Applique le loadout + personnage sauvegardés dans le GameInstance
  if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance()))
  {
    // --- Swap de personnage si une classe spécifique a été choisie ---
    TSubclassOf<ARevenantOpsCharacter> ChosenClass = *GI->PendingCharacter.CharacterClass
        ? TSubclassOf<ARevenantOpsCharacter>(*GI->PendingCharacter.CharacterClass)
        : nullptr;
    APawn* CurrentPawn = GetPawn();

    if (ChosenClass && CurrentPawn && !CurrentPawn->IsA(ChosenClass))
    {
      // Spawn le bon personnage à la position du pawn actuel
      FTransform SpawnTransform = CurrentPawn->GetActorTransform();
      FActorSpawnParameters SP;
      SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

      if (ARevenantOpsCharacter* NewChar = GetWorld()->SpawnActor<ARevenantOpsCharacter>(ChosenClass, SpawnTransform, SP))
      {
        UnPossess();
        CurrentPawn->Destroy();
        Possess(NewChar);
        CurrentPawn = NewChar;
      }
    }

    // --- Applique les talents + loadout ---
    if (ARevenantOpsCharacter* MercChar = Cast<ARevenantOpsCharacter>(GetPawn()))
    {
      // Talents depuis l'écran de sélection (remplace les éventuels talents BP par défaut)
      if (GI->PendingCharacter.Talents.Num() > 0)
      {
        MercChar->AssignedTalents = GI->PendingCharacter.Talents;
        MercChar->ApplyTalents();
      }

      // Armes du loadout (extraites de l'inventaire par défaut)
      TArray<TSubclassOf<AWeaponBase>> Loadout;
      if (GI->PendingPrimaryWeapon)   Loadout.Add(GI->PendingPrimaryWeapon);
      if (GI->PendingSecondaryWeapon && GI->PendingSecondaryWeapon != GI->PendingPrimaryWeapon)
          Loadout.Add(GI->PendingSecondaryWeapon);
      if (Loadout.Num() > 0)
      {
        MercChar->SetDefaultWeaponClasses(Loadout);
        MercChar->SpawnDefaultWeapons();
      }

      // Inventaire complet du personnage (items non-armes)
      if (GI->PendingCharacter.DefaultInventory.Num() > 0)
      {
        for (const FInventoryItem& Item : GI->PendingCharacter.DefaultInventory)
        {
          if (Item.Type != EInventoryItemType::Weapon && !Item.IsEmpty())
            MercChar->AddItemToInventory(Item);
        }
      }
    }
  }

  // Show HUD
  if (!HUDWidget && HUDWidgetClass) {
    HUDWidget = CreateWidget<URevenantOpsHUD>(this, HUDWidgetClass);
  }
  if (HUDWidget && !HUDWidget->IsInViewport()) {
    HUDWidget->AddToViewport(0);
  }

  // Re-activer l'input du pawn (DisableInput peut avoir été appelé lors d'une mort précédente)
  if (APawn* P = GetPawn()) {
    P->EnableInput(this);
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

      // Quand toutes les vagues sont terminées → fin de match (victoire)
      WS->OnAllWavesCompleted.AddDynamic(this, &ARevenantOpsPlayerController::OnAllWavesCompleted);

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

  // ── Death animation ───────────────────────────────────────────────────────
  // Jouer le montage de mort s'il est assigné. Le GameOver est déclenché après
  // la durée de l'anim (ou immédiatement si aucun montage n'est assigné).
  float DeathAnimDelay = 0.f;
  if (ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(GetPawn()))
  {
    if (Char->DeathMontage)
    {
      const float MontageLen = Char->PlayAnimMontage(Char->DeathMontage);
      if (MontageLen > 0.f)
      {
        DeathAnimDelay = MontageLen;
      }
    }
  }

  // End the match après la death anim (ou immédiatement)
  auto TriggerEndMatch = [this]()
  {
    if (AMercenairesGameState* GS = GetWorld()->GetGameState<AMercenairesGameState>())
    {
      GS->EndMatch();
    }
  };

  if (DeathAnimDelay > 0.f)
  {
    FTimerHandle DeathMatchTimer;
    GetWorldTimerManager().SetTimer(
        DeathMatchTimer, TriggerEndMatch, DeathAnimDelay, /*bLoop=*/false);
  }
  else
  {
    TriggerEndMatch();
  }
}

void ARevenantOpsPlayerController::OnAllWavesCompleted()
{
  // Désactiver l'input joueur
  if (APawn* P = GetPawn())
  {
    P->DisableInput(this);
  }

  // Terminer le match en victoire — unbind d'abord pour éviter le double-save
  if (AMercenairesGameState* GS = GetWorld()->GetGameState<AMercenairesGameState>())
  {
    GS->OnMatchStateChanged.RemoveDynamic(this, &ARevenantOpsPlayerController::OnMatchEnded);
    GS->EndMatch();
  }
  ShowGameOverScreen(true); // victoire
}

void ARevenantOpsPlayerController::OnMatchEnded(bool bIsActive) {
  if (bIsActive) {
    return;
  }

  ShowGameOverScreen(false); // mort
}

void ARevenantOpsPlayerController::ShowGameOverScreen(bool bVictory) {
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
    // Sauvegarde par niveau pour le leaderboard de sélection
    FString LbSlot = TEXT("Leaderboard_Default");
    if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance()))
    {
        if (!GI->PendingLevel.MapName.IsNone())
            LbSlot = FString::Printf(TEXT("Leaderboard_%s"), *GI->PendingLevel.MapName.ToString());
    }
    ULeaderboardWidget::SaveScoreStatic(this, FinalScore, TotalKills, BestCombo, LbSlot);
    UE_LOG(LogTemp, Warning, TEXT("[GameOver] Score=%d Kills=%d BestCombo=%d"),
        FinalScore, TotalKills, BestCombo);
  } else {
    UE_LOG(LogTemp, Error, TEXT("[GameOver] GameState null — stats will show 0!"));
  }

  // Stopper la musique in-game
  if (GameMusicComponent)
  {
    GameMusicComponent->FadeOut(1.5f, 0.f);
    GameMusicComponent = nullptr;
  }

  // Stopper le combat — les ennemis/spawner s'arrêtent
  UGameplayStatics::SetGamePaused(this, true);

  if (GameOverWidgetClass) {
    GameOverWidgetInstance =
        CreateWidget<UGameOverWidget>(this, GameOverWidgetClass);
    if (GameOverWidgetInstance) {
      GameOverWidgetInstance->AddToViewport(10);
      GameOverWidgetInstance->ShowResults(FinalScore, TotalKills, BestCombo, bVictory);
      SetShowMouseCursor(true);
      SetInputMode(FInputModeUIOnly());
    }
  }
}

void ARevenantOpsPlayerController::ShowLeaderboard() {
  DoTransition([this]()
  {
    ClearFlowWidgets();

    if (LeaderboardWidgetClass) {
      LeaderboardWidgetInstance =
          CreateWidget<ULeaderboardWidget>(this, LeaderboardWidgetClass);
      if (LeaderboardWidgetInstance) {
        // Charger le slot du niveau courant
        FString LbSlot = TEXT("Leaderboard");
        if (URevenantOpsGameInstance* GI = Cast<URevenantOpsGameInstance>(GetGameInstance()))
          if (!GI->PendingLevel.MapName.IsNone())
            LbSlot = FString::Printf(TEXT("Leaderboard_%s"), *GI->PendingLevel.MapName.ToString());
        LeaderboardWidgetInstance->SetSaveSlot(LbSlot);
        LeaderboardWidgetInstance->LoadScores();
        LeaderboardWidgetInstance->AddToViewport(10);
        ActiveMenu = LeaderboardWidgetInstance.Get();
        SetShowMouseCursor(true);
        SetInputMode(FInputModeUIOnly());
      }
    }
  });
}

void ARevenantOpsPlayerController::ClearFlowWidgets() {
  if (TitleScreenWidget) {
    TitleScreenWidget->RemoveFromParent();
    TitleScreenWidget = nullptr;
  }
  if (OptionsWidgetInstance) {
    OptionsWidgetInstance->RemoveFromParent();
    OptionsWidgetInstance = nullptr;
  }
  if (LevelSelectWidgetInstance) {
    LevelSelectWidgetInstance->RemoveFromParent();
    LevelSelectWidgetInstance = nullptr;
  }
  if (CharacterSelectWidgetInstance) {
    CharacterSelectWidgetInstance->RemoveFromParent();
    CharacterSelectWidgetInstance = nullptr;
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
