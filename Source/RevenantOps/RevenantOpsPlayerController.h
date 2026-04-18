// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/MenuTypes.h"
#include "RevenantOpsPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class URevenantOpsHUD;
class UTitleScreenWidget;
class ULevelSelectWidget;
class UCharacterSelectWidget;
class ULoadoutWidget;
class UGameOverWidget;
class ULeaderboardWidget;
class UInventoryWidget;
class AMercenairesGameState;
class AWeaponBase;
class UHealthComponent;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ARevenantOpsPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** HUD widget class to spawn */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<URevenantOpsHUD> HUDWidgetClass;

	/** Pointer to the HUD widget instance */
	UPROPERTY()
	TObjectPtr<URevenantOpsHUD> HUDWidget;

	// ========== MERCENAIRES FLOW WIDGETS ==========

	/** Title screen widget class */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TSubclassOf<UTitleScreenWidget> TitleScreenClass;

	/** Level selection widget class */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TSubclassOf<ULevelSelectWidget> LevelSelectWidgetClass;

	/** Character selection widget class */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TSubclassOf<UCharacterSelectWidget> CharacterSelectWidgetClass;

	/** Loadout selection widget class */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TSubclassOf<ULoadoutWidget> LoadoutWidgetClass;

	/** Game over widget class */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TSubclassOf<UGameOverWidget> GameOverWidgetClass;

	/** Leaderboard widget class */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TSubclassOf<ULeaderboardWidget> LeaderboardWidgetClass;

	/** Weapon classes available for loadout selection */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TArray<TSubclassOf<AWeaponBase>> AvailableWeaponClasses;

	/** Levels available for selection — configure in BP */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TArray<FLevelInfo> AvailableLevels;

	/** Characters available for selection — configure in BP */
	UPROPERTY(EditAnywhere, Category = "UI|Mercenaires")
	TArray<FCharacterInfo> AvailableCharacters;

	UPROPERTY()
	TObjectPtr<UTitleScreenWidget> TitleScreenWidget;

	UPROPERTY()
	TObjectPtr<ULevelSelectWidget> LevelSelectWidgetInstance;

	UPROPERTY()
	TObjectPtr<UCharacterSelectWidget> CharacterSelectWidgetInstance;

	UPROPERTY()
	TObjectPtr<ULoadoutWidget> LoadoutWidgetInstance;

	UPROPERTY()
	TObjectPtr<UGameOverWidget> GameOverWidgetInstance;

	UPROPERTY()
	TObjectPtr<ULeaderboardWidget> LeaderboardWidgetInstance;

	// ========== INVENTORY ==========

	/** Inventory widget class (assign WBP_Inventory in BP) */
	UPROPERTY(EditAnywhere, Category = "UI|Inventory")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> InventoryWidgetInstance;

	bool bInventoryOpen = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Called when a local player is assigned — safe point for CreateWidget */
	virtual void ReceivedPlayer() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:

	// ========== GAME FLOW ==========

	/** Shows the title screen */
	UFUNCTION(BlueprintCallable, Category = "Mercenaires|Flow")
	void ShowTitleScreen();

	/** Shows the level selection screen */
	UFUNCTION(BlueprintCallable, Category = "Mercenaires|Flow")
	void ShowLevelSelectScreen();

	/** Shows the character selection screen */
	UFUNCTION(BlueprintCallable, Category = "Mercenaires|Flow")
	void ShowCharacterSelectScreen();

	/** Shows the loadout selection screen */
	UFUNCTION(BlueprintCallable, Category = "Mercenaires|Flow")
	void ShowLoadoutScreen();

	/** Starts the match after loadout confirmation */
	UFUNCTION(BlueprintCallable, Category = "Mercenaires|Flow")
	void StartMercenairesMatch();

	/** Shows the game over screen */
	UFUNCTION(BlueprintCallable, Category = "Mercenaires|Flow")
	void ShowGameOverScreen();

	/** Shows the leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Mercenaires|Flow")
	void ShowLeaderboard();

protected:

	/** Handler for level chosen */
	UFUNCTION()
	void OnLevelChosen(FLevelInfo LevelInfo);

	/** Handler for character chosen */
	UFUNCTION()
	void OnCharacterChosen(FCharacterInfo CharacterInfo);

	/** Handler for back from level select */
	UFUNCTION()
	void OnLevelSelectBack();

	/** Handler for back from character select */
	UFUNCTION()
	void OnCharacterSelectBack();

	/** Handler for loadout confirmation */
	UFUNCTION()
	void OnLoadoutConfirmed(TSubclassOf<AWeaponBase> Primary,
	                        TSubclassOf<AWeaponBase> Secondary);

	/** Handler for match end */
	UFUNCTION()
	void OnMatchEnded(bool bIsActive);

	/** Handler for player death — ends match immediately */
	UFUNCTION()
	void OnPlayerDied(UHealthComponent* HealthComp, const AController* InstigatedBy, AActor* DamageCauser);

	/** Removes all flow widgets from screen */
	void ClearFlowWidgets();

	/** Opens or closes the RE5-style inventory */
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void ToggleInventory();

	/** Handler when player uses an item in the inventory */
	UFUNCTION()
	void OnInventoryItemUsed(int32 SlotIndex);

	/** Guard against double loadout confirmation */
	bool bLoadoutConfirmed = false;

	/** Guard against double UI initialization in ReceivedPlayer */
	bool bFlowInitialized = false;

public:

	/** Returns the HUD widget instance */
	UFUNCTION(BlueprintCallable, Category = "UI")
	URevenantOpsHUD* GetHUDWidget() const { return HUDWidget; }

};
