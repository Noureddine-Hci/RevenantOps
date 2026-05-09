// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UI/SplashScreenWidget.h"
#include "MainMenuGameMode.generated.h"

class UAudioComponent;

/**
 *  GameMode du menu principal.
 *  1. Affiche la sequence d'intro (logos moteur, studio, partenaires)
 *  2. Lance la musique de menu en fondu entrant
 *  3. Affiche TitleScreenWidget (via PlayerController) quand la sequence est terminee
 *
 *  Setup dans BP_MainMenuGameMode :
 *    - SplashWidgetClass → WBP_SplashScreen
 *    - SplashSequence    → remplir les entrees (Logo, SubText, durees)
 *    - MenuMusic         → sound asset (loop)
 */
UCLASS()
class REVENANTOPS_API AMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMainMenuGameMode();
    virtual void BeginPlay() override;

    // ── SPLASH ───────────────────────────────────────────────────────────────

    /** Classe du widget d'intro — assigner WBP_SplashScreen */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Splash")
    TSubclassOf<USplashScreenWidget> SplashWidgetClass;

    /**
     *  Sequence d'ecrans d'intro definie directement dans le GameMode BP.
     *  Ordre conseille : Unreal Engine → Studio → Partenaires.
     *  Laisser vide pour sauter directement au menu.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Splash")
    TArray<FSplashEntry> SplashSequence;

    // ── AUDIO ─────────────────────────────────────────────────────────────────

    /** Musique jouee en boucle sur les menus */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Audio")
    USoundBase* MenuMusic = nullptr;

    /** Volume cible de la musique (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Audio",
              meta = (ClampMin = 0.f, ClampMax = 1.f))
    float MenuMusicVolume = 0.7f;

    /**
     *  Duree du fondu entrant de la musique (secondes).
     *  0 = demarrage immediat au volume cible.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Audio",
              meta = (ClampMin = 0.f, ClampMax = 10.f))
    float MusicFadeInDuration = 2.f;

protected:
    UFUNCTION()
    void OnSplashDone();

    void ShowMainMenu(bool bInstant = false);
    void StartMenuMusic();

private:
    UPROPERTY()
    TObjectPtr<USplashScreenWidget> SplashWidget  = nullptr;

    UPROPERTY()
    TObjectPtr<UAudioComponent>     MusicComponent = nullptr;
};
