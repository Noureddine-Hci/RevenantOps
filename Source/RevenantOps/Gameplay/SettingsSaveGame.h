// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingsSaveGame.generated.h"

/**
 *  Settings persistés entre sessions (volumes, FOV, sensibilité).
 *  Chargé au boot via URevenantOpsGameInstance::Init() et exposé via getters.
 */
UCLASS()
class REVENANTOPS_API USettingsSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // ── AUDIO ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
    float MusicVolume  = 0.8f;

    UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
    float SFXVolume    = 1.0f;

    // ── GAMEPLAY ──────────────────────────────────────────────────────────

    /** Sensibilité souris (1.0 = neutre, > 1 plus rapide) */
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
    float MouseSensitivity = 1.0f;

    /** Sensibilité ADS (multiplicateur appliqué quand le joueur vise) */
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
    float ADSSensitivityMultiplier = 0.6f;

    /** FOV de base (entre 70 et 110) */
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
    float FieldOfView = 90.f;

    /** Inverser l'axe Y de la souris */
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
    bool bInvertY = false;

    // ── VIDEO ─────────────────────────────────────────────────────────────

    /** 0=Low, 1=Medium, 2=High, 3=Epic */
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Video")
    int32 GraphicsQuality = 2;

    /** Fullscreen / windowed (0=Fullscreen, 1=WindowedFullscreen, 2=Windowed) */
    UPROPERTY(BlueprintReadWrite, Category = "Settings|Video")
    int32 WindowMode = 1;
};
