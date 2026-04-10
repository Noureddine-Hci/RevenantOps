// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RevenantOpsPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class URevenantOpsHUD;
class UTitleScreenWidget;
class ULoadoutWidget;
class UGameOverWidget;
class ULeaderboardWidget;
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

	UPROPERTY()
	TObjectPtr<UTitleScreenWidget> TitleScreenWidget;

	UPROPERTY()
	TObjectPtr<ULoadoutWidget> LoadoutWidgetInstance;

	UPROPERTY()
	TObjectPtr<UGameOverWidget> GameOverWidgetInstance;

	UPROPERTY()
	TObjectPtr<ULeaderboardWidget> LeaderboardWidgetInstance;

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

	/** Guard against double loadout confirmation */
	bool bLoadoutConfirmed = false;

	/** Guard against double UI initialization in ReceivedPlayer */
	bool bFlowInitialized = false;

public:

	/** Returns the HUD widget instance */
	UFUNCTION(BlueprintCallable, Category = "UI")
	URevenantOpsHUD* GetHUDWidget() const { return HUDWidget; }

};
