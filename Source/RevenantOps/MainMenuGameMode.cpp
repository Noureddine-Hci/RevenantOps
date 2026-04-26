// Copyright RevenantOps. All Rights Reserved.
#include "MainMenuGameMode.h"
#include "Kismet/GameplayStatics.h"

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

    if (MenuMusic)
        UGameplayStatics::SpawnSound2D(this, MenuMusic, MenuMusicVolume, 1.f, 0.f, nullptr, false, true);
}
