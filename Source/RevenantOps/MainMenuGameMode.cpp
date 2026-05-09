// Copyright RevenantOps. All Rights Reserved.
#include "MainMenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "RevenantOpsPlayerController.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    DefaultPawnClass = nullptr;
    HUDClass         = nullptr;
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    // S'assurer que le jeu n'est pas en pause (peut persister depuis le niveau de jeu)
    UGameplayStatics::SetGamePaused(this, false);

    // Splash sequence si configurée, sinon menu direct
    if (SplashWidgetClass && SplashSequence.Num() > 0)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            SplashWidget = CreateWidget<USplashScreenWidget>(PC, SplashWidgetClass);
            if (SplashWidget)
            {
                // Passer la séquence directement sur la UPROPERTY du widget
                SplashWidget->SplashSequence = SplashSequence;
                SplashWidget->OnSequenceDone.AddDynamic(this, &AMainMenuGameMode::OnSplashDone);
                SplashWidget->AddToViewport(20);
            }
        }
    }
    else
    {
        ShowMainMenu();
        StartMenuMusic();
    }
}

void AMainMenuGameMode::OnSplashDone()
{
    if (SplashWidget)
    {
        SplashWidget->RemoveFromParent();
        SplashWidget = nullptr;
    }
    ShowMainMenu(/*bInstant=*/true);
    StartMenuMusic();
}

void AMainMenuGameMode::ShowMainMenu(bool bInstant)
{
    if (ARevenantOpsPlayerController* PC =
            Cast<ARevenantOpsPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        PC->ShowTitleScreen(bInstant);
    }
}

void AMainMenuGameMode::StartMenuMusic()
{
    if (!MenuMusic) return;

    UAudioComponent* AC = UGameplayStatics::SpawnSound2D(
        this, MenuMusic, /*VolumeMultiplier=*/0.f,
        /*PitchMultiplier=*/1.f, /*StartTime=*/0.f,
        /*ConcurrencySettings=*/nullptr,
        /*bPersistAcrossLevelTransition=*/false,
        /*bAutoDestroy=*/false);

    if (AC)
    {
        MusicComponent = AC;
        if (MusicFadeInDuration > 0.f)
            AC->FadeIn(MusicFadeInDuration, MenuMusicVolume);
        else
            AC->SetVolumeMultiplier(MenuMusicVolume);
    }
}
